#include "systemcalls.h"

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
    int sys_ret = system(cmd);
    // printf("\033[1;34m return value is  <%i>\033[0m \n", sys_ret);
    
    if ((cmd == NULL) && (sys_ret == 0))
    {
        return false;
    }
    if (sys_ret == -1 )
    {
        return false;
    }
    else if (sys_ret == 127)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    bool return_value = false;
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
        
    }
    command[count] = NULL;
    // 
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    // command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/


    int pid = fork();

    if (pid == -1)
    {   
        // failed (you are still in parent process)
        return false;
    }
    // if child
    else if (pid == 0)
    {
        if (execv(command[0],command) == -1)
        {
            exit(-1); // need to exit with a non 0 value so that waitpid can capture this exit as a failure
        }
    }
    else
    {
        // parent process
        // printf("\033[1;34m inside parent <%i>\033[0m\n",pid);
        
        int status;
        if (waitpid (pid, &status, 0) == -1)
        {
            return_value = false;
        }
        else if (WIFEXITED(status))
        {
            if (WEXITSTATUS(status) == 0)
            {
                return_value = true;
            }
            else
            {
                return_value = false;
            }
        }
        else
        {
            return_value = false;
        }
    }
    
    va_end(args);

    return return_value;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    // command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/
    bool return_value = false;
    int pid = fork();
    int fd = open(outputfile, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (pid == -1)
    {   
        // failed (you are still in parent process)
        return false;
    }
    else if (pid == 0)
    {
        //inside child
        if (dup2(fd, STDOUT_FILENO)<0)
        {
            return false;
        }
        if (execv(command[0],command) == -1)
        {
            exit(-1); // need to exit with a non 0 value so that waitpid can capture this exit as a failure
        }
    }
    else
    {
        // parent process
        int status;
        if (waitpid (pid, &status, 0) == -1)
        {
            return_value = false;
        }
        else if (WIFEXITED(status))
        {
            if (WEXITSTATUS(status) == 0)
            {
                return_value = true;
            }
            else
            {
                return_value = false;
            }
        }
        else
        {
            return_value = false;
        }
    }
    
    va_end(args);

    return return_value;

 
}
