#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <signal.h>
#include "LineParser.h"
#include <fcntl.h>
#include <ctype.h>

#define MAX_PATH 2048

//  LAB C
#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0
#define HISTLEN 20
#define MAX_BUF 200

// struct for LAB C
typedef struct process
{
    cmdLine *cmd;         /* the parsed command line*/
    pid_t pid;            /* the process id that is running the command*/
    int status;           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next; /* next process in chain */
} process;

void execute(cmdLine *pCmdLine);
void debug_process(pid_t pid, cmdLine *pcmdline);
void execute_pipeline(cmdLine *pipeline);

// LAB C function :

// part3a
void addProcess(process **process_list, cmdLine *cmd, pid_t pid);
void printProcessList(process **process_list);
// part3b
void freeProcessList(process *process_list);
void updateProcessList(process **process_list);
void updateProcessStatus(process *process_list, int pid, int status);
// part 4
void add2History(char *unpcmdline);
void print_history();
char *get_history_command(int index);

// LAB C var
process *ProcessList = NULL;

char history[HISTLEN][MAX_BUF];
int newest = -1;
int oldest = 0;
int history_count = 0;
//////////

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

        if (strcmp(parsedCmd->arguments[0], "history") != 0 && 
        strcmp(parsedCmd->arguments[0], "!!") != 0 && 
        (parsedCmd->arguments[0][0] != '!' || !isdigit(parsedCmd->arguments[0][1])))
    {
        add2History(buffer);
    }
        
        if (parsedCmd == NULL)
        {
            continue;
        }
        if (strncmp(parsedCmd->arguments[0], "quit", 4) == 0)
        {
            if (ProcessList)
            {
                freeProcessList(ProcessList);
            }

            debug_process(getpid(), parsedCmd);

            break;
        }

        // Part 2 Lab C , handling the "|" case and proc command

        else if (strncmp(parsedCmd->arguments[0], "history", 7) == 0) {
            print_history();
        }

        else if (strncmp(parsedCmd->arguments[0], "!!", 2) == 0) {
            if (history_count == 0) {
                fprintf(stderr, "No commands in history\n");
                continue;
            }
            strcpy(buffer, history[newest]);
            parsedCmd = parseCmdLines(buffer);
            if(parsedCmd->next != NULL){
                execute_pipeline(parsedCmd);
            }
            else execute(parsedCmd);
        }

        else if (buffer[0] == '!' && isdigit(buffer[1])) {
            int index = atoi(&buffer[1]);
            char *cmd_from_history = get_history_command(index);
            if (cmd_from_history) {
                strcpy(buffer, cmd_from_history);
                parsedCmd = parseCmdLines(buffer);
                if(parsedCmd->next != NULL){
                execute_pipeline(parsedCmd);
            }
            else execute(parsedCmd);

            } else {
                continue;
            }
        }

        
        else if (parsedCmd->next != NULL)
        {
            if (parsedCmd->outputRedirect != NULL)
            {
                fprintf(stderr, "Error: Output redirection on the left-hand side of a pipeline is not allowed.\n");
                exit(EXIT_FAILURE);
            }
            if (parsedCmd->next->inputRedirect != NULL)
            {
                fprintf(stderr, "Error: Input redirection on the right-hand side of a pipeline is not allowed.\n");
                exit(EXIT_FAILURE);
            }
            execute_pipeline(parsedCmd);
        }


        // ============================================================================== //

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
    }
    return 0;
}

void execute(cmdLine *pCmdLine)
{
    // task 2 (blast , alarm)
    if (strcmp(pCmdLine->arguments[0], "alarm") == 0)
    { // alarm comand
        if (pCmdLine->argCount != 2)
        {
            fprintf(stderr, "Usage: alarm \n");
            return;
        }
        pid_t pid = atoi(pCmdLine->arguments[1]);
        if (kill(pid, SIGCONT) == -1)
        {
            perror("kill SIGCONT");
        }
        else
        {
            printf("Process %d continued.\n", pid);
        }
        // LAB C
        
        updateProcessStatus(ProcessList, pid, RUNNING);

        return;
    }
    if (strcmp(pCmdLine->arguments[0], "blast") == 0)
    { // blast comand
        if (pCmdLine->argCount != 2)
        {
            fprintf(stderr, "Usage: blast \n");
            return;
        }

        pid_t pid = atoi(pCmdLine->arguments[1]);

        if (kill(pid, SIGTERM) == -1)
        {
            perror("kill SIGTERM");
        }
        else
        {
            printf("Process %d terminated.\n", pid);
        }

        // LAB C Part 3b
        updateProcessStatus(ProcessList, pid, TERMINATED);

        return;
    }

    else if (strcmp(pCmdLine->arguments[0], "sleep") == 0)
        { // sleep command
            if (pCmdLine->argCount != 2)
            {
                fprintf(stderr, "Usage: sleep <process id>\n");
                exit(EXIT_FAILURE);
            }

            pid_t pid = atoi(pCmdLine->arguments[1]);

            if (kill(pid, SIGTSTP) == -1)
            {
                perror("kill SIGTSTP");
            }
            else
            {
                
                printf("Process %d suspended.\n", pid);
                updateProcessStatus(ProcessList, pid, SUSPENDED);
                return;
            }
        }
        else if (strncmp(pCmdLine->arguments[0], "proc", 4) == 0)
        {
            printProcessList(&ProcessList);
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
            file_descriptor = open(pCmdLine->inputRedirect, O_RDWR | O_CREAT, 0777);
            if (file_descriptor == -1)
            {
                perror("failed to open \n");
                exit(1);
            }
            if (dup2(file_descriptor, STDIN_FILENO) == -1)
            {
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
            if (dup2(file_descriptor, STDOUT_FILENO) == -1)
            {
                perror("dup2 input file");
                _exit(1);
            }
            close(file_descriptor);
        }

        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
        {
            fprintf(stderr, "Failed to execute \n");
            _exit(1);
        }
    }
    if (pid > 0)
    {
        if (pCmdLine->blocking)
        {
            waitpid(pid, &status_process, 0);
        }
        // adding the looper to the process list
        addProcess(&ProcessList, pCmdLine, pid);
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

// Part2 Lab C
void execute_pipeline(cmdLine *pipeline)
{
    int pipe_fd[2];
    pid_t child1, child2;

    if (pipeline == NULL || pipeline->next == NULL)
    {
        fprintf(stderr, "Error: Invalid pipeline structure.\n");
        return;
    }

    if (pipe(pipe_fd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    child1 = fork();

    if (child1 == -1)
    {
        perror("fork error (child1)");
        exit(EXIT_FAILURE);
    }

    if (child1 == 0)
    {
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);
        execvp(pipeline->arguments[0], pipeline->arguments);
        perror("execvp child 1");
        exit(EXIT_FAILURE);
    }

    child2 = fork();

    if (child2 == -1)
    {
        perror("fork error (child2)");
        exit(EXIT_FAILURE);
    }

    if (child2 == 0)
    {
        close(pipe_fd[1]);
        dup2(pipe_fd[0], STDIN_FILENO);
        close(pipe_fd[0]);
        execvp(pipeline->next->arguments[0], pipeline->next->arguments);
        perror("execvp child 2");
        exit(EXIT_FAILURE);
    }

    close(pipe_fd[0]);
    close(pipe_fd[1]);

    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);

    
    return;
}


// LAB C part 3a

void addProcess(process **process_list, cmdLine *cmd, pid_t pid)
{
    process *new_process = (process *)malloc(sizeof(process));
    new_process->cmd = cmd;
    new_process->pid = pid;
    new_process->status = RUNNING;
    new_process->next = *process_list;
    *process_list = new_process;
}

void printProcessList(process **process_list)
{
    printf("PID\t\tCommand\t\tStatus\n");

    // there is no process to print
    if (process_list == NULL || *process_list == NULL)
    {
        return;
    }

    char *curr_status;
    process *curr = *process_list;
    process *prev = NULL;

    while (curr != NULL)
    {
        if (curr->status == RUNNING)
        {
            curr_status = "RUNNING";
        }
        else if (curr->status == SUSPENDED)
        {
            curr_status = "SUSPENDED";
        }
        else if (curr->status == TERMINATED)
        {
            curr_status = "TERMINATED";
        }

        printf("%d\t\t%s\t\t%s\n", curr->pid, curr->cmd->arguments[0], curr_status);

        // updating the function (part3b)

        if (curr->status == TERMINATED)
        {
            if (prev == NULL)
            {
                *process_list = curr->next;
                freeCmdLines(curr->cmd);
                free(curr);
                curr = *process_list;
            }
            else
            {
                prev->next = curr->next;
                freeCmdLines(curr->cmd);
                free(curr);
                curr = prev->next;
            }
        }
        else
        {
            prev = curr;
            curr = curr->next;
        }
    }
}

// LAB C part 3b
void freeProcessList(process *process_list)
{
    process *curr = process_list;
    process *next;

    while (curr != NULL)
    {
        next = curr->next;
        freeCmdLines(curr->cmd);
        free(curr);
        curr = next;
    }
}

// use them in the main and test 27.6.2024 ...

void updateProcessList(process **process_list)
{
    process *curr = *process_list;
    int status;
    pid_t result;

    while (curr != NULL)
    {
        result = waitpid(curr->pid, &status, WNOHANG);
        if (result == -1)
        {
            perror("waitpid");
        }
        else if (result > 0)
        {
            if (WIFEXITED(status) || WIFSIGNALED(status))
            {
                curr->status = TERMINATED;
            }
            else if (WIFSTOPPED(status))
            {
                curr->status = SUSPENDED;
            }
            else if (WIFCONTINUED(status))
            {
                curr->status = RUNNING;
            }
        }
        curr = curr->next;
    }
}

void updateProcessStatus(process *process_list, int pid, int status)
{
    process *curr = process_list;
    while (curr != NULL)
    {
        if (curr->pid == pid)
        {
            curr->status = status;
            return;
        }
        curr = curr->next;
    }
}

void add2History(char *unpcmdline)
{
    newest = (newest + 1) % HISTLEN;
    strncpy(history[newest], unpcmdline, MAX_BUF - 1);
    history[newest][MAX_BUF - 1] = '\0'; // Ensure null termination

    if (history_count < HISTLEN)
    {
        history_count++;
    }
    else
    {
        oldest = (oldest + 1) % HISTLEN;
    }
}

void print_history()
{
    int index = oldest;
    for (int i = 0; i < history_count; i++)
    {
        printf("%d %s\n", i + 1, history[index]);
        index = (index + 1) % HISTLEN;
    }
}

char *get_history_command(int index)
{
    if (index < 1 || index > history_count)
    {
        fprintf(stderr, "Invalid history index\n");
        return NULL;
    }
    return history[(oldest + index - 1) % HISTLEN];
}
