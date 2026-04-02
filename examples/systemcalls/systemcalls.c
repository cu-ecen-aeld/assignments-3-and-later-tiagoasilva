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
    int result = system(cmd);
    
    if(cmd == NULL)
    {
        if (result == 0)
        {
            printf("Shell is not available.");
            return false;
        }
        else
        {
            printf("Shell is available.");
            return true;
        }
    }
    else
    {
        if (result == -1)
        {
            printf("A child process could not be created, or its status could not be retrieved. Errno: %d. Strerror output: %s\n", errno, strerror(errno));
            return false;
        }
        else
        {
            if (WIFSIGNALED(result) && (WTERMSIG(result) == SIGINT || WTERMSIG(result) == SIGQUIT))
            {
                printf("Signal SIGINT or SIGQUIT received. Terminating the process.\n");
                return false;
            }
            else
            {
                if (WIFEXITED(result))
                {
                    int exit_code = WEXITSTATUS(result);
                    printf("Command exited with code: %d\n", exit_code);
                    return true;
                }
                else
                {
                    printf("Command did not terminate normally.\n");
                    return false;
                }
            }
        }
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
    command[count] = command[count];

	bool parent = true;
    bool result = true;
	pid_t pid = fork();
    if (pid < 0)
    {
        printf("Fork failed. errno: %d. strerror output: %s\n", errno, strerror(errno));
        result = false;
    }
    else
    {
        if ( pid == 0 )
        {
            parent = false;
            execv(command[0], command);
            printf("An error occured. Returning status value on child: %d\n", errno);
            exit(errno);
        }
        if (parent)
        {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status))
            {
                int exit_code = WEXITSTATUS(status);
                if (exit_code == 0)
                {
                    printf("The process executed with SUCCESS (exit code 0).\n");
                }
                else
                {
                    printf("The process exited normally but with errno %d. strerror output: %s\n", exit_code, strerror(exit_code));
                    result = false;
                }
            }
            else
            {
                if (WIFSIGNALED(status))
                {
                    printf("The process was killed by a signal.\n");
                    result = false;
                } 
                else 
                {
                    printf("The process did not exit normally.\n");
                    result = false;
                }
            }
        }
    }

    va_end(args);

    return result;
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
    command[count] = command[count];

    bool result = true;
    int pid;
    int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0644);
    if (fd < 0)
    { 
        perror("open failed");
        result = false;
    }
    switch (pid = fork()) 
    {
        case -1:
            perror("fork failed"); 
            result = false;
            break;
        case 0:
            if (dup2(fd, 1) < 0) 
            { 
                perror("dup2 failed"); 
                abort();
            }
            close(fd);
            execv(command[0], command);
            perror("An error occured on execv. Returning status value on child.");
            exit(errno);
        default:
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status))
            {
                int exit_code = WEXITSTATUS(status);
                if (exit_code == 0)
                {
                    printf("The process executed with SUCCESS (exit code 0).\n");
                }
                else
                {
                    perror("The process exited normally but with error status.");
                    result = false;
                }
            }
            else
            {
                if (WIFSIGNALED(status))
                {
                    perror("The process was killed by a signal.");
                    result = false;
                } 
                else 
                {
                    perror("The process did not exit normally.");
                    result = false;
                }
            }
            close(fd);
    }


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

    va_end(args);

    return result;
}
