# socket server

# define variables 
CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -Wall -g
CFLAGS += -I$(CURDIR)/../aesd-char-driver/
CFLAGS += -DUSE_AESD_CHAR_DEVICE

TARGET ?= aesdsocket
LDFLAGS?= -lpthread -lrt
SRCS = aesdsocket.c
SRCS += signal_handler.c
SRCS += socket_core.c
SRCS += linkedlist.c

# HEADERS is documentation only
HEADERS = signal_handler.h
HEADERS += socket_core.h
HEADERS += queue.h
HEADERS += linkedlist.h
HEADERS += ../aesd-char-driver/aesd_ioctl.h
OBJS = $(SRCS:.c=.o)
BUILD_COMMAND=$(CC) $(CFLAGS) $(LDFLAGS) 

# the default target 
all: $(TARGET)

# link objects
$(TARGET): $(OBJS)
	$(BUILD_COMMAND) -o $@ $^

# wild card for all objects and .c files
%.o: %.c
	$(BUILD_COMMAND) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean