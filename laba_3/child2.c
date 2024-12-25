#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

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
    int shm_fd = shm_open("child_shm", O_RDWR, 0666);
    if (shm_fd == -1) {
        const char message[] = "SharedMemoryError: Failed to open shared memory";
        write(STDERR_FILENO, message, sizeof(message));
        exit(EXIT_FAILURE);
    }

    void *addr = mmap(NULL, sizeof(int) + sizeof(char) * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (addr == MAP_FAILED) {
        const char message[] = "MemoryMappingError: Failed to map shared memory";
        write(STDERR_FILENO, message, sizeof(message));
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    int *len_string = (int *) addr;
    char *string = (char *)(len_string + 1);

    remove_extra_spaces(string, len_string);

    string[*len_string] = '\n';
    (*len_string)++;

    munmap(addr, sizeof(int) + sizeof(char) * 1024);
    close(shm_fd);

    return 0;
}
