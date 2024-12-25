#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
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

    to_lower(string, *len_string);

    munmap(addr, sizeof(int) + sizeof(char) * 1024);
    close(shm_fd);

    return 0;
}
