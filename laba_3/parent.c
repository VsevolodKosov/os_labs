#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

typedef enum {
    OK,
    MemoryError
} status_code;

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

int main() {
    const char* args[] = {
        "./child1", NULL
    };

    int len_string;
    char *string;
    if (get_string(&string, &len_string) == MemoryError){
        const char message[] = "MemoryError: Failed to allocate memory";
        write(STDERR_FILENO, message, sizeof(message));
        exit(EXIT_FAILURE);
    }

    int shm_fd = shm_open("child_shm", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        const char message[] = "SharedMemoryError: Failed to create shared memory";
        write(STDERR_FILENO, message, sizeof(message));
        exit(EXIT_FAILURE);
    }

    ftruncate(shm_fd, sizeof(int) + len_string * sizeof(char));

    void *addr = mmap(NULL, sizeof(int) + len_string * sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (addr == MAP_FAILED) {
        const char message[] = "MemoryMappingError: Failed to map shared memory";
        write(STDERR_FILENO, message, sizeof(message));
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    int *shared_len_string = (int *) addr;
    char *shared_string = (char *)(shared_len_string + 1);

    *shared_len_string = len_string;
    memcpy(shared_string, string, len_string);

    free(string);

    int pid1 = fork();
    if (pid1 == -1){
        const char message[] = "ProcessError: Failed to create a new process";
        write(STDERR_FILENO, message, sizeof(message));
        exit(EXIT_FAILURE);
    }
    else if (pid1 == 0){
        execv("./child1", (char **) args);
    }

    int pid2 = fork();
    if (pid2 == -1){
        const char message[] = "ProcessError: Failed to create a new process";
        write(STDERR_FILENO, message, sizeof(message));
        exit(EXIT_FAILURE);
    }
    else if (pid2 == 0){
        execv("./child2", (char **) args);
    }

    wait(NULL);
    wait(NULL);

    int *final_len = (int *) addr;
    char *final_string = (char *)(final_len + 1);
    
    write(STDOUT_FILENO, final_string, *final_len);

    munmap(addr, sizeof(int) + len_string * sizeof(char));
    close(shm_fd);
    shm_unlink("child_shm");

    return 0;
}
