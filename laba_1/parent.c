#include <stdint.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

typedef enum{
    OK,
    MemoryError
}status_code;

status_code get_string(char **string, int *len_string) {
    char c;
    *len_string = 0;
    int capacity_string = 2;
    *string = (char *)malloc(capacity_string * sizeof(char)); 
    if (*string == NULL) {
        return MemoryError;  
    }

    do {
        c = getchar();
        if (c == EOF || c == '\n') break;  
        (*string)[*len_string] = c;       
        (*len_string)++;                 

        if (*len_string == capacity_string) {
            capacity_string *= 2;
            char *tmp_string = (char *)realloc(*string, capacity_string * sizeof(char));
            if (tmp_string == NULL) { 
                free(*string); 
                *string = NULL;
                return MemoryError;
            }
            *string = tmp_string;  
        }
    } while (c != EOF && c != '\n');  

    (*string)[*len_string] = '\0';  
    return OK;
}


int main(){
    const char* args[] = {
        "./child.",
        NULL
    };

    int len_string;
    char *string;
    if (get_string(&string, &len_string) == MemoryError){
        const char message[] = "MemoryError: Failed to allocate memory";
        write(STDERR_FILENO, message, sizeof(message));
        exit(EXIT_FAILURE);
    }

    int pipe_between_child1[2]; int pipe_between_child2[2];
    pipe(pipe_between_child1); pipe(pipe_between_child2);

    int pid1 = fork();
    if (pid1 == -1){
        const char message[] = "ProcessError: Failed to create a new process";
        write(STDERR_FILENO, message, sizeof(message));
        exit(EXIT_FAILURE);
    }
    else if (pid1 == 0){
        dup2(pipe_between_child1[0], STDIN_FILENO);
        execv("./child1", (char **) args);
    }
    else{
        write(pipe_between_child1[1], &len_string, sizeof(int));
        write(pipe_between_child1[1], string, sizeof(char) * len_string);
        free(string); string = NULL;
    }

    int pid2 = fork();
    if (pid2 == -1){
        const char message[] = "ProcessError: Failed to create a new process";
        write(STDERR_FILENO, message, sizeof(message));
        exit(EXIT_FAILURE);
    }
    else if (pid2 == 0){
        dup2(pipe_between_child2[1], STDOUT_FILENO);
        execv("./child2", (char**) args);
    }
    else{
        read(pipe_between_child2[0], &len_string, sizeof(int));
        string = (char *)malloc(len_string * sizeof(char));
        if (string == NULL){
            const char message[] = "MemoryError: Failed to allocate memory";
            write(STDERR_FILENO, message, sizeof(message));
            exit(EXIT_FAILURE);
        }
        read(pipe_between_child2[0], string, len_string * sizeof(char));
        write(STDOUT_FILENO, string, len_string * sizeof(char));
        free(string); string = NULL;

        int child_status;
		wait(&child_status);

		if (child_status != EXIT_SUCCESS) {
			const char msg[] = "Error: Child exited with error\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			exit(child_status);
        }  
    }
    return 0;
}