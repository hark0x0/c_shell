#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <mcheck.h>

#include "parser.h"
#include "shell.h"

/**
 * Program that simulates a simple shell.
 * The shell covers basic commands, including builtin commands 
 * (cd and exit only), standard I/O redirection and piping (|). 
 
 */

#define MAX_DIRNAME 100
#define MAX_COMMAND 1024
#define MAX_TOKEN 128

/* Functions to implement, see below after main */
int execute_cd(char** words);
int execute_nonbuiltin(simple_command *s);
int execute_simple_command(simple_command *cmd);
int execute_complex_command(command *cmd);


int main(int argc, char** argv) {

        char cwd[MAX_DIRNAME];           /* Current working directory */
        char command_line[MAX_COMMAND];  /* The command */
        char *tokens[MAX_TOKEN];         /* Command tokens (program name, 
                                          * parameters, pipe, etc.) */

        while (1) {

                /* Display prompt */
                getcwd(cwd, MAX_DIRNAME-1);
                printf("%s> ", cwd);

                /* Read the command line */
                fgets(command_line, MAX_COMMAND, stdin);
                /* Strip the new line character */
                if (command_line[strlen(command_line) - 1] == '\n') {
                        command_line[strlen(command_line) - 1] = '\0';
                }

                /* Parse the command into tokens */
                parse_line(command_line, tokens);

                /* Check for empty command */
                if (!(*tokens)) {
                        continue;
                }

                /* Construct chain of commands, if multiple commands */
                command *cmd = construct_command(tokens);
                //print_command(cmd, 0);
    
                int exitcode = 0;
                if (cmd->scmd) {
                        exitcode = execute_simple_command(cmd->scmd);
                        if (exitcode == -1) {
                                break;
                        }
                }
                else {
                        exitcode = execute_complex_command(cmd);
                        if (exitcode == -1) {
                                break;
                        }
                }
                release_command(cmd);
        }
    
        return 0;
}


/**
 * Changes directory to a path specified in the words argument;
 * For example: words[0] = "cd"
 *              words[1] = "csc209/assignment3/"
 * Your command should handle both relative paths to the current 
 * working directory, and absolute paths relative to root,
 * e.g., relative path:  cd csc209/assignment3/
 *       absolute path:  cd /u/bogdan/csc209/assignment3/
 */
int execute_cd(char** words) {

        /** 
         * TODO: 
         * The first word contains the "cd" string, the second one contains 
         * the path.
         * Check possible errors:
         * - The words pointer could be NULL, the first string or the second 
         *   string could be NULL, or the first string is not a cd command
         * - If so, return an EXIT_FAILURE status to indicate something is 
         *   wrong.
         */

        //error checking


        if (is_relative(words[1])){

                //according to strcat, the '\0' char is replaced, so do we still need to do MAX_DIRNAME-1 to append? 
                char *cwd;
                cwd = malloc(sizeof(MAX_DIRNAME));
                getcwd(cwd, strlen(cwd));
                strcat(cwd, words[1]);

                //change directory
                chdir(cwd);
        }
        else{
                chdir(words[1]);
        } 
        return 0;


        /**
         * TODO:          * The safest way would be to first 
         * The safest way would be to first determine if the path is relative 
         * or absolute (see is_relative function provided).
         * - If it's not relative, then simply change the directory to the path 
         * specified in the second word in the array.
         * - If it's relative, then make sure to get the current working 
         * directory, append the path in the second word to the current working
         * directory and change the directory to this path.
         * Hints: see chdir and getcwd man pages.
         * Return the success/error code obtained when changing the directory.
         */
         
}


/**
 * Executes a program, based on the tokens provided as 
 * an argument.
 * For example, "ls -l" is represented in the tokens array by 
 * 2 strings "ls" and "-l", followed by a NULL token.
 * The command "ls -l | wc -l" will contain 5 tokens, 
 * followed by a NULL token. 
 */
int execute_command(char **tokens) {

        /**
         * TODO: execute a program, based on the tokens provided.
         * The first token is the command name, the rest are the arguments 
         * for the command. 
         * Hint: see execlp/execvp man pages.
         * 
         * - In case of error, make sure to use "perror" to indicate the name
         *   of the command that failed.
         *   You do NOT have to print an identical error message to what would 
         *   happen in bash.
         *   If you use perror, an output like: 
         *      my_silly_command: No such file of directory 
         *   would suffice.
         * Function returns only in case of a failure (EXIT_FAILURE).
         */

        int exec_nonb = execvp(*tokens, tokens);

        if(exec_nonb == -1){
                perror(*tokens);
        }

        //return exec_nonb;

        return 0;
}


/**
 * Executes a non-builtin command.
 */
int execute_nonbuiltin(simple_command *s) {
        /**
         * TODO: Check if the in, out, and err fields are set (not NULL),
         * and, IN EACH CASE:
         * - Open a new file descriptor (make sure you have the correct flags,
         *   and permissions);
         * - redirect stdin/stdout/stderr to the corresponding file.
         *   (hint: see dup2 man pages).
         * - close the newly opened file descriptor in the parent as well. 
         *   (Avoid leaving the file descriptor open across an exec!) 
         * - finally, execute the command using the okens (see execute_command
         *   function above).
         * This function returns only if the execution of the program fails.
         */
        if(s->in != NULL){
        //create a file descriptor
                int fd_in;

                //open a file descriptor, set flags and permissions
                fd_in = open(s->in, O_RDONLY);

                //error checking
                if(fd_in < 0){
                        return 1;
                }
                //redirect stdin to in by using dup2 call
                if(dup2(fd_in, fileno(stdin)) < 0){
                        return 1;
                }
                //close the file descriptor
                close(fd_in);
        }


        if(s->out != NULL){
                //create a file descriptor
                int fd_out;
                fd_out = open(s->out, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);

                //error checking
                if(fd_out < 0){
                        //exit(EXIT_FAILURE);
                        return 1;
                }
                //redirect to standard output using dup2
                if(dup2(fd_out, fileno(stdout)) < 1){
                        return 1;
                }
                //close the file descriptor
                close(fd_out);
        }


        if (s->err != NULL){
                //open a file descriptor
                int fd_err;
                fd_err = open(s->err, O_RDWR);


                if (dup2(fd_err, fileno(stderr)) < 2){
                        exit(EXIT_FAILURE);
                }
                close(fd_err);
        }

        if (execute_command(s->tokens) < 0){
                exit(EXIT_FAILURE);
        }
        exit(0);
        return 0;
}



/**
 * Executes a simple command (no pipes).
 */
int execute_simple_command(simple_command *cmd) {

        /**
         * TODO: 
         * Check if the command is builtin.
         * 1. If it is, then handle BUILTIN_CD (see execute_cd function provided) 
         *    and BUILTIN_EXIT (simply exit with an appropriate exit status).
         * 2. If it isn't, then you must execute the non-builtin command. 
         * - Fork a process to execute the nonbuiltin command 
         *   (see execute_nonbuiltin function above).
         * - The parent should wait for the child.
         *   (see wait man pages).
         */


        pid_t pid;
        int status;

        if(cmd->builtin == 1){
                //execute a change directory command
                execute_cd(cmd->tokens);
        }
        else if(cmd->builtin == 2){
                //exit the shell
                exit(1);
        }
        else if(cmd->builtin == 0){

                //fork process to execute the nonbuiltin
                pid = fork();

                //execute_nonbuiltin in the child process 
                if(pid == 0){
                        int nonbuiltin = execute_nonbuiltin(cmd);
                        if(nonbuiltin){
                        //      perror("execute_nonbuiltin failure");
                                exit(EXIT_FAILURE);
                        }
                        exit(0);
                }

                //wait for the child
                else if (pid > 0){
                        wait(&status);
                        if(WIFEXITED(status)){
                                return 0;
                        }
                        else
                                exit(EXIT_FAILURE);
                exit(0);
                }
        }
        return 0;
}


/**
 * Executes a complex command.  A complex command is two commands chained 
 * together with a pipe operator.
 */
int execute_complex_command(command *c) {

        /**
         * TODO:
         * Check if this is a simple command, using the scmd field.
         * Remember that this will be called recursively, so when you encounter
         * a simple command you should act accordingly.
         * Execute nonbuiltin commands only. If it's exit or cd, you should not 
         * execute these in a piped context, so simply ignore builtin commands. 
         */


        if(c->scmd != NULL){
                if(c->scmd->builtin == 0){
                        execute_nonbuiltin(c->scmd);
                }
                exit(0);
        }

        //else{
                //execute_complex_command(c);
        //}

        /** 
         * Optional: if you wish to handle more than just the 
         * pipe operator '|' (the '&&', ';' etc. operators), then 
         * you can add more options here. 
         */

        if (!strcmp(c->oper, "|")) {

                /**
                 * TODO: Create a pipe "pfd" that generates a pair of file 
                 * descriptors, to be used for communication between the 
                 * parent and the child. Make sure to check any errors in 
                ] * creating the pipe.
                */
                int pfd[2];
                if (pipe(pfd) == -1) {
                        perror("pipe");
                        exit(1);
                }

                int status1;
                int status2;

                //TODO: error check



                /**
                 * TODO: Fork a new process.
                 * In the child:
                 *  - close one end of the pipe pfd and close the stdout 
                 * file descriptor.
                 *  - connect the stdout to the other end of the pipe (the 
                 * one you didn't close).
                 *  - execute complex command cmd1 recursively. 
                 * In the parent: 
                 *  - fork a new process to execute cmd2 recursively.
                 *  - In child 2:
                 *     - close one end of the pipe pfd (the other one than 
                 *       the first child), and close the standard input file 
                 *       descriptor.
                 *     - connect the stdin to the other end of the pipe (the 
                 *       one you didn't close).
                 *     - execute complex command cmd2 recursively. 
                 *  - In the parent:
                 *     - close both ends of the pipe. 
                 *     - wait for both children to finish.
                 */

                pid_t pid = fork();

                if(pid == 0){
                        //close read end of the pipe
                        close(pfd[0]);
                        dup2(pfd[1], fileno(stdout));
                        execute_complex_command(c->cmd1);
                        exit(0);
                } 

                else if(pid > 0){

                        int pid2 = fork();

                        if(pid2 == 0){
                                close(pfd[1]);
                                dup2(pfd[0], fileno(stdin));
                                execute_complex_command(c->cmd2);
                                exit(1);
                        }
                        else if(pid2 > 0){
                                close(pfd[0]);
                                close(pfd[1]);
                                if (wait(&status1) == -1) {
                                        // error
                                } else {
                                        if (WIFEXITED(status1) != 0) {
                                                // normal exit
                                        } else {
                                                printf("child did not exit normally\n");
                                        }

                                }

                                if (wait(&status2) == -1) {
                                        // error
                                } else {
                                        if (WIFEXITED(status2) != 0) {
                                                // normal exit
                                        } else {
                                                printf("child did not exit normally\n");
                                        }

                                }

                        }

                        else{
                                perror("FORK ERROR OMG");
                        }
                }

        }
        return 0;
}
