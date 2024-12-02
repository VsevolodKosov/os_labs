#include <stdint.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void remove_extra_spaces(char *str, int *length) {
    int i = 0, j = 0;
    int in_space = 0;

    while (str[i] != '\0') {
        if (str[i] == ' ') {
            if (!in_space && i != 0) {
                str[j++] = ' ';
                in_space = 1;
            }
        } else {
            str[j++] = str[i];
            in_space = 0;
        }
        i++;
    }

    str[j] = '\0';
    *length = j;
}

int main() {
    int fd = open("childrens_pipe", O_RDONLY);
    if (fd == -1) {
        const char message[] = "OpenFileError: Failed to open file fifo";
        write(STDERR_FILENO, message, sizeof(message));
        exit(EXIT_FAILURE);
    }

    int len_string; 
    if (read(fd, &len_string, sizeof(int)) != sizeof(int)) {
        const char message[] = "ReadFileError: Failed to read length from fifo";
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

    if (read(fd, string, len_string) != len_string) {
        const char message[] = "ReadFileError: Failed to read string from fifo";
        write(STDERR_FILENO, message, sizeof(message));
        free(string);
        close(fd);
        exit(EXIT_FAILURE);
    }

    remove_extra_spaces(string, &len_string);

    string[len_string] = '\n';
    len_string++;

    if (write(STDOUT_FILENO, &len_string, sizeof(int)) == -1 || 
        write(STDOUT_FILENO, string, len_string) == -1) {
        const char message[] = "WriteFileError: Failed to write result to stdout";
        write(STDERR_FILENO, message, sizeof(message));
        free(string);
        close(fd);
        exit(EXIT_FAILURE);
    }

    free(string);
    close(fd);
    return 0;
}
