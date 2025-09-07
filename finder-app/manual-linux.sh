#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u
RED='\033[0;31m'
ORANGE='\033[0;33m'
GREEN='\033[0;32m'
NC='\033[0m' 
REPO_DIR=${PWD}
OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi
cores=$(nproc)
echo "I will be using ${cores} cores when using make command"
mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    # make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules # skipped as its done by default for ours specific arch
    make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs

fi

echo "Adding the Image in outdir"
cp -r ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}/Image
echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
cd "$OUTDIR"
mkdir -p rootfs
cd rootfs

mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
make -j$(nproc) distclean
make -j$(nproc)  defconfig
make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
else
    cd busybox
fi

# TODO: Make and install busybox
make -j$(nproc)  CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install


echo "Library dependencies"
cd ${OUTDIR}/rootfs/
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
# ---------------------------------------------------------------------------
# -------------------------Start of Assisted work ---------------------------
# ---------------------------------------------------------------------------
# got the sysroot correct copy from this repo
# https://github.com/cu-ecen-aeld/assignments-3-and-later-Rajshekhar1208/blob/main/finder-app/manual-linux.sh
# to help fix the can't run /bin/sh issue when running QEMU
SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)

cp ${SYSROOT}/lib/ld-linux-aarch64.so.1 ${OUTDIR}/rootfs/lib
cp ${SYSROOT}/lib64/libm.so.6 ${OUTDIR}/rootfs/lib64
cp ${SYSROOT}/lib64/libresolv.so.2 ${OUTDIR}/rootfs/lib64
cp ${SYSROOT}/lib64/libc.so.6 ${OUTDIR}/rootfs/lib64
# ---------------------------------------------------------------------------
# -------------------------END of Assisted work -----------------------------
# ---------------------------------------------------------------------------

# TODO: Make device nodes
sudo mknod  -m 666 ${OUTDIR}/rootfs/dev/null c 1 3
sudo mknod  -m 600 ${OUTDIR}/rootfs/dev/console c 5 1


# TODO: Clean and build the writer utility
# using cross_compile to make sure writter works on arm64 linux
cd "${FINDER_APP_DIR}"; make clean ; make CROSS_COMPILE=${CROSS_COMPILE}
# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
sudo cp -r  ${FINDER_APP_DIR}/* ${OUTDIR}/rootfs/home
cd ..
cp -r conf ${OUTDIR}/rootfs/

echo -e "${GREEN}DONE copying files${NC}"


# TODO: Chown the root directory
cd ${OUTDIR}/rootfs/
sudo chown -R root:root *
sudo chmod +x ${OUTDIR}/rootfs/bin

# TODO: Create initramfs.cpio.gz
# -H format -v verbose -o use  ARCHIVE-NAME instead of start output
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
echo -e "${GREEN}DONE with creating cpio file${NC}"
gzip -f  ${OUTDIR}/initramfs.cpio
echo -e "${GREEN}DONE with creating gzipping initramfs.cpio.gz${NC}"


