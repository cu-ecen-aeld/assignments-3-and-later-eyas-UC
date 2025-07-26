#include <stdio.h>
#include <syslog.h>

int main(int argc, char ** argv)
{
    int ret_val = 0;
    openlog("log_id", LOG_PID, LOG_USER);
    if (argc == 3)
    {
        
        // printf("file is <%s>\n string to be written is <%s>\n", (char*)argv[1], (char*)argv[2]);
        FILE *fptr;
        syslog(LOG_DEBUG ,"Writing %s to %s",(char*)argv[2],(char*)argv[1]);
        
        // Open a file in writing mode
        fptr = fopen((char*)argv[1], "w");
        if (fptr == NULL)
        {
            syslog(LOG_ERR ,"Could not open %s",(char*)argv[1]);
            // perror("perror returned\n");
            ret_val =  1;
        }
        fputs((char*)argv[2], fptr);
        fclose(fptr);
    }
    else if (argc < 3)
    {
        // perror("less than 2 arguments");
        syslog(LOG_ERR ,"less than 2 arguments");
        ret_val = 1;
    }
    else
    {
        // perror("more than 2 arguments");
        syslog(LOG_ERR ,"more than 2 arguments");
        ret_val = 1;
    }
    closelog();
    return ret_val;
}