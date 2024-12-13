#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

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
    // Получаем доступ к разделяемой памяти и семафору
    sem_t *sem = sem_open("/semaphore", 0);
    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }

    int len_string;
    char *string = (char *)malloc(1024);
    if (string == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    sem_wait(sem);
    remove_extra_spaces(string, &len_string);
    sem_post(sem);

    free(string);
    return 0;
}
