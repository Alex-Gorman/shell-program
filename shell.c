
/* Kristopher Lancaster 	KRL872 11261005
# Alex Gorman 		 AJG279 11194069 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define NUM_LOCATIONS 6

/* String of the current working directory */
char* currentDirectory() {
    size_t len;
    char* curDir;
    curDir = (char*) malloc(1024*(sizeof(char)));
    getcwd(curDir, 1024);
    len = strlen(curDir);
    curDir[len] = ':';
    curDir[len+1] = '~';
    curDir[len+2] = '$';
    curDir[len+3] = '\0';
    return curDir;
}

/* Get the string of the commands from the command line entered by user */
char* read_command_from_line() {
    char* command_line = NULL;
    size_t command_line_size = 0;
    getline(&command_line, &command_line_size, stdin);
    return command_line;
}

/* Tokenize the string entered by the user */
char** tokenize_string(char* string) {
    int i = 0;
    char** tokens = (char**) malloc (64*(sizeof(string)));
    const char delim[10] = " \n";

    tokens[i] = strtok(string, delim);

    while (tokens[i] != NULL) {
        i = i + 1;
        tokens[i] = strtok(NULL, delim);
    }

    return tokens;
}

/* Built in command to exit the shell */
void shell_exit() {
    exit(0);
}

/* Built in command to change directory in the shell */
void shell_cd(char** commands) {
    int cd;
    cd = chdir(commands[1]);
    if (cd != 0) printf("No Such Directory\n");
}

/* Check if given command is built in, and if it is execute it */
int is_built_in_command(char** command) {

    int x = 0;
    int case_number = 0;

    char* commands[2] = {"cd", "exit"};

    for (x = 0; x < 2; x = x + 1) {
        if (strcmp(command[0], commands[x]) == 0) {
            break;
        }
        case_number = case_number + 1;
        if (x == 1) case_number = -1;
    }

    switch (case_number) {
        case 0:
            shell_cd(command);
            return 1;
        case 1:
            shell_exit();
            return 1;
        default:
            ;
    }

    return 0;
}

/* Execute a single command */
void shell_execute_command(char** commands) {
    /* int number_of_locations = 6; */
    int x = 0;
    const char* standard_locations[] = {"/usr/local/sbin/", "/usr/local/bin/", "/usr/sbin/", "/usr/bin/", "/sbin/", "/bin/"};
    char potential_path[128] = "";
    int path_exists;
    pid_t child_pid;

    for (x = 0; x < NUM_LOCATIONS; x = x + 1) {
        strcat(potential_path, standard_locations[x]);
        strcat(potential_path, commands[0]);
        path_exists = access(potential_path, F_OK);

        if (path_exists == 0) {
            break;
        } else {
            memset(potential_path, 0, sizeof(potential_path));
        }

        if (x == 5) {
            printf("Error: No current file or directory\n");
            return;
        }

    }

    child_pid = fork();

    if (child_pid == 0) {
        execv(potential_path, commands);
    } else if (child_pid > 0) {

        while(wait(NULL)>0);
    } else {
        ;
    }


}

/* Check if there is a pipe character */
int shell_check_if_pipe(char** commands) {
    int i = 0;
    while(commands[i] != NULL) {
        if ((strcmp(commands[i], "|\0")) == 0) {
            return 1;
        }
        i = i + 1;
    }
    return 0;
}

/* Execute the pipe commands */
void shell_execute_pipe_command(char** commands1, char** commands2) {
    
    int fd[2];
    pid_t p1;
    pid_t p2;
    const char* standard_locations[] = {"/usr/local/sbin/", "/usr/local/bin/", "/usr/sbin/", "/usr/bin/", "/sbin/", "/bin/"};

    char potential_path_one[128] = "";
    char potential_path_two[128] = "";

    int x = 0;

    int path_exists;

    for (x = 0; x < NUM_LOCATIONS; x = x + 1) {
        strcat(potential_path_one, standard_locations[x]);
        strcat(potential_path_one, commands1[0]);
        path_exists = access(potential_path_one, F_OK);

        if (path_exists == 0) {
            break;
        } else {
            memset(potential_path_one, 0, sizeof(potential_path_one));
        }

        if (x == 5) {
            printf("Error: No current file or directory\n");
            return;
        }

    }

    for (x = 0; x < NUM_LOCATIONS; x = x + 1) {
        strcat(potential_path_two, standard_locations[x]);
        strcat(potential_path_two, commands2[0]);
        path_exists = access(potential_path_two, F_OK);

        if (path_exists == 0) {
            break;
        } else {
            memset(potential_path_two, 0, sizeof(potential_path_two));
        }

        if (x == 5) {
            printf("Error: No current file or directory\n");
            return;
        }

    }


    pipe(fd);

    p1 = fork();
    if (p1 == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        /* if (execvp(commands1[0], commands1) < 0) {
                printf("Error in p1");
            } */
        execv(potential_path_one, commands1);
        printf("!!! p1\n");
        printf("%s\n", commands1[0]);
    } else {
        p2 = fork();

        if (p2 < 0) {
            printf("Error could not create second child");
        }

        if (p2 == 0) {
            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            /* if (execvp(commands2[0], commands2) < 0) {
                printf("Error in p2");
            } */
            execv(potential_path_two, commands2);
        }
        else {
            /* while(wait(NULL)>0); */
            /* waitpid(p1, 0, 0);
            waitpid(p2, 0, 0); */
            close(fd[1]);
            wait(NULL);
            wait(NULL);
        }
    }
    return;
    
    
}

int main() {    

    while (1) {
        char* line;
        char** tokens;
        char* curDir = currentDirectory();

        int i = 0;
        int j = 0;
        int compare;
        char** args1;
        char** args2;
        char* char_ptr;
        char pipeline[] = "|\0";
        int pipe_counter = 0;

        printf("%s ",curDir);
        free(curDir);

        line = read_command_from_line();

        char_ptr = (char*)line;

        while(*char_ptr != 0){
            if (*char_ptr == '|') pipe_counter = pipe_counter + 1;
            ++char_ptr;
        }

        if (pipe_counter > 1) {
            printf("Error: cannot have more than one pipe command\n");
        } else {
            tokens = tokenize_string(line);

            args1 = (char**) malloc (64*(sizeof(tokens)));
            args2 = (char**) malloc (64*(sizeof(tokens)));

            if (is_built_in_command(tokens)) {
                ;
            } else if (shell_check_if_pipe(tokens)) {
                compare = strcmp(tokens[0], pipeline);

                while (tokens[i] != NULL && compare != 0) {
                    args1[i] = tokens[i];
                    compare = strcmp(tokens[i+1], pipeline);
                    i = i + 1;
                }

                i = i + 1;

                while (tokens[i] != NULL) {
                    args2[j] = tokens[i];
                    i = i + 1;
                    j = j + 1;
                }

                shell_execute_pipe_command(args1, args2);
            } else {
                shell_execute_command(tokens);
            }
        }
    }
    return 1;
}