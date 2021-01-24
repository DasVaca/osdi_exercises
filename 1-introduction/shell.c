#include <stdio.h>      
#include <unistd.h>     // pid_t, fork()
#include <string.h>     // strnlen(), strcmp()
#include <stdlib.h>     // malloc()
#include <sys/wait.h>   // waitpid()

#define BUFFSIZE 256

const char * builtins[] = {"cd", "history", "exit"};


int ntoken(char * s, char token) {
    /* return how many of 'token' are in 's' */

    if (!s) {
        return 0;
    }

    int ntk = 0;
    int slen = strlen(s);

    if (slen < 0) {
        return -1;
    }

    for (int i = 0; i < slen; i++) {
        if (s[i] == token) {
            ntk++;
        }
    }

    return ntk;
}

int itoken(char * s, char token) {
    if (!s) {
        return -1;
    }

    static int i = 0;
    
    int slen = strlen(s);

    if (i + 1 >= slen) {
        return -1;
    }

    for (i = i + 1;i < slen; i++) {
        if (s[i] == token) {
            break;
        }
    }
    
    return i;
}

int main()
{
    char buffer[BUFFSIZE] = {0};
    
    while (1) {
        printf("$ ");
        fgets(buffer, BUFFSIZE-1, stdin);

        pid_t pid = fork();

        if (pid > 0) {
            // parent
            int wstatus;

            waitpid(pid, &wstatus, 0);

            if (WIFSIGNALED(wstatus)) {
                printf("\nProgram terminated with signal %i.\n", WTERMSIG(wstatus));
            }

            if (WIFEXITED(wstatus)) {
                printf("\nProgram terminated with exit code %i.\n", WEXITSTATUS(wstatus));
            }

        } else if (pid == 0) {
            // child
            if (strncmp(buffer, "exit", strlen("exit")) == 0) {
                break;
            }

            /* remove breakline */
            buffer[strlen(buffer) - 1] = '\0';
            
            /* Now we are going to split the string by spaces */

            int ntk = ntoken(buffer, ' ');

            if (ntk == -1) {
                puts("Bad input");
                break;
            }
            
            char ** args;

            args = malloc((ntk + 2) * sizeof(char *));
            args[ntk + 1] = NULL;

            int itk;            // token index
            int ac = 0;         // args counter
            int litk = 0;       // last token index 

            while ((itk = itoken(buffer, ' ')) != -1) {

                /*  We store the string contained between litk and itk
                 *  0 ---- litk ----- itk ---- BUFFSIZE
                 *  That distance is itkn - litkn
                 */
                args[ac++] = strndup(buffer + litk, itk - litk);
                litk = itk + 1;
            }

            for (int i = 0; i < ntk+2; i++) {
                printf("%s.", args[i]);
            }

            if (execvp(args[0], args) == -1) {
                puts("\nCouldn't execute command\n");    
                break;
            } 
        } else {
            // error
            puts("Couldn't fork");
            break;
        }
    }

    return 0;
}
