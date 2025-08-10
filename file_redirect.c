#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


int main ()
{
    int fd = open("redirected_print.txt",O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd == -1)
    {
        printf("error opening file %s\n", strerror(errno));
        perror("open");
    }
    int pid = fork();
    if (pid == -1)
    {
        perror("could not fork");
    }
    else if (pid == 0 )
    {
        // inside child
        // make standard output a copy of the file descriptor we created
        if (dup2(fd,fileno(stdout)) < 0 )
        {
            perror("duplication failed!");
        }
        else
        {
            printf("I think this should be printed in the file now!\n");
            close(fd);
        }
        printf("I think this should be printed in the file now!\n");
        printf("this is inside the child process will this is alos be redirected?!\n");

    }
    else
    {
        printf("this is inside the parent will this is alos be redirected?!\n");
        
        // inside parent
        close(fd);
    }

}