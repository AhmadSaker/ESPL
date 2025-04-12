#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    int pipe_fd[2];
    pid_t child1, child2;

    // creating pipe
    if (pipe(pipe_fd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // forking the first child
    fprintf(stderr, "(parent_process>forking...)\n");

    child1 = fork();

    if (child1 == -1)
    {
        perror("fork error (child1)");
        exit(EXIT_FAILURE);
    }

    if (child1 == 0)
    {

        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe...)\n");

        close(STDOUT_FILENO); // closing the standart output
        dup(pipe_fd[1]);      // duplicating
        close(pipe_fd[1]);    // closing the duplicated file

        fprintf(stderr, "(child1>going to execute cmd: ls -l)\n");
        // Executing "ls -l"
        char *args[] = {"ls", "-l", NULL}; // NULL pointer to terminate the array
        execvp(args[0], args);             // executing
        perror("error in exicuting ls -l");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "(parent_process>created process with id: %d)\n", child1);

    fprintf(stderr, "(parent_process>closing the write end of the pipe...)\n");

    // closing the write end in the parent process ...
    close(pipe_fd[1]);

    fprintf(stderr, "(parent_process>forking...)\n");
    // forking child2
    child2 = fork();

    if (child2 == -1)
    {
        perror("fork error (child2)");
        exit(EXIT_FAILURE);
    }

    if (child2 == 0)
    {

        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe...)\n");

        close(STDIN_FILENO); // closing the standart input
        dup(pipe_fd[0]);     // duplicating
        close(pipe_fd[0]);   // closing the duplicated file

        fprintf(stderr, "(child2>going to execute cmd: tail -n 2)\n");

        char *args2[] = {"tail", "-n", "2", NULL};
        execvp(args2[0], args2); // executing the tail -n 2
        perror("error while (tail -n 2) ");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "(parent_process>created process with id: %d)\n", child2);
    fprintf(stderr, "(parent_process>closing the read end of the pipe...)\n");

    // closing the read end in the parent process ...
    close(pipe_fd[0]);

    fprintf(stderr, "(parent_process>waiting for child processes to terminate...)\n");

    // waiting
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);

    fprintf(stderr, "(parent_process>exiting...)\n");

    return 0;
}
