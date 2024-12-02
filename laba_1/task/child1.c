#include <stdint.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

void to_lower(char *string, int len_string) {
    for (int i = 0; i < len_string; ++i) {
        if (string[i] >= 'A' && string[i] <= 'Z') {
            string[i] += 32;
        }
    }
}

int main() {
    int fd = open("childrens_pipe", O_WRONLY);
    if (fd == -1) {
        const char message[] = "OpenFileError: Failed to open file fifo";
        write(STDERR_FILENO, message, sizeof(message));
        exit(EXIT_FAILURE);
    }

    int len_string; 
    if (read(STDIN_FILENO, &len_string, sizeof(int)) != sizeof(int)) {  
        const char message[] = "ReadFileError: Failed to read length from stdin";
        write(STDERR_FILENO, message, sizeof(message));
        close(fd);
        exit(EXIT_FAILURE);
    }

    char *string = (char *)malloc(len_string * sizeof(char));
    if (string == NULL) {
        const char message[] = "MemoryError: Failed to allocate memory";
        write(STDERR_FILENO, message, sizeof(message));
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (read(STDIN_FILENO, string, len_string) != len_string) {  
        const char message[] = "ReadFileError: Failed to read string from stdin";
        write(STDERR_FILENO, message, sizeof(message));
        free(string);
        close(fd);
        exit(EXIT_FAILURE);
    }

    to_lower(string, len_string);

    if (write(fd, &len_string, sizeof(int)) == -1 || write(fd, string, len_string) == -1) {
        const char message[] = "WriteFileError: Failed to write in fifo";
        write(STDERR_FILENO, message, sizeof(message));
        free(string);
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
    free(string);
    return 0;
}
