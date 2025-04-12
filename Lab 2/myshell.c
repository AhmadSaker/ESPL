#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <signal.h>
#include "LineParser.h"
#include <fcntl.h>

#define MAX_PATH 2048

void execute(cmdLine *pCmdLine);
void debug_process(pid_t pid, cmdLine *pcmdline);

bool debug_mode = false;
int main(int argc, char **argv)
{
    char path[MAX_PATH];
    char buffer[MAX_PATH];
    cmdLine *parsedCmd = NULL;
    // checking that there is no "-d" 
    for (int i = 0; i < argc; i++)
    {
        if (strncmp(argv[i], "-d", 2) == 0)
        {
            debug_mode = true; 
        }
    }

    while (1)
    {
        if (getcwd(path, MAX_PATH) != NULL) 
        {
            printf("%s #> ", path);
        }
        fgets(buffer, MAX_PATH, stdin);
        parsedCmd = parseCmdLines(buffer);
        if (parsedCmd == NULL)
        {
            continue;
        }
        if (strncmp(parsedCmd->arguments[0], "quit", 4) == 0)
        {
            debug_process(getpid(), parsedCmd);
            break;
        }
        else if (strncmp(parsedCmd->arguments[0], "cd", 2) == 0)
        {
            debug_process(getpid(), parsedCmd);
            if (chdir(parsedCmd->arguments[1]) == -1)
            {
                perror("chdir has failed\n");
            }
        }
        else
        {
            execute(parsedCmd);
        }
        freeCmdLines(parsedCmd); 
    }
    return 0;
}

void execute(cmdLine *pCmdLine)
{
    
    //task 2 (blast , alarm)
        if (strcmp(pCmdLine->arguments[0], "alarm") == 0) { //alarm comand 
        if (pCmdLine->argCount != 2) {
            fprintf(stderr, "Usage: alarm \n");
            return;
        }
        pid_t pid = atoi(pCmdLine->arguments[1]);
        if (kill(pid, SIGCONT) == -1) {
            perror("kill SIGCONT");
        } else {
            printf("Process %d continued.\n", pid);
        }
        return;
    }
    if (strcmp(pCmdLine->arguments[0], "blast") == 0) { // blast comand 
        if (pCmdLine->argCount != 2) {
            fprintf(stderr, "Usage: blast \n");
            return;
        }
        pid_t pid = atoi(pCmdLine->arguments[1]);
        if (kill(pid, SIGTERM) == -1) {
            perror("kill SIGTERM");
        } else {
            printf("Process %d terminated.\n", pid);
        }
        return;
    }
    
    pid_t pid = fork();
    int status_process;
    int file_descriptor;
    
    if (pid == 0) // child.
    {   
        // task 3
        if (pCmdLine->inputRedirect != NULL)
        {
            
            file_descriptor = open(pCmdLine->inputRedirect, O_RDWR | O_CREAT, 0777); // I got help here
            if (file_descriptor == -1)
            {
                perror("failed to open \n");
                exit(1);
            }
            if (dup2(file_descriptor, STDIN_FILENO) == -1){
                perror("dup2 input file");
                _exit(1);
            }
            close(file_descriptor);
        }

        if (pCmdLine->outputRedirect != NULL)
        {
            
            file_descriptor = open(pCmdLine->outputRedirect, O_RDWR | O_CREAT, 0777);
            if (file_descriptor == -1)
            {
                perror("failed to open \n");
                exit(1);
            }
            if (dup2(file_descriptor, STDOUT_FILENO) == -1){
                perror("dup2 input file");
                _exit(1);
            }
            close(file_descriptor);
        }


        
        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
        {
            fprintf(stderr, "Failed to execute");
            _exit(1);
        }
    }
    if (pid > 0)
    { // parent
        if (pCmdLine->blocking)
        {
            waitpid(pid, &status_process, 0);
        }
    }
    debug_process(getpid(), pCmdLine);
}

void debug_process(pid_t pid, cmdLine *pCmdLine)
{
    if (debug_mode)
    {
        fprintf(stderr, "> Process ID: %d.\n> Command: %s.\n", pid, pCmdLine->arguments[0]);
    }
}


