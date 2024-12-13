#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <semaphore.h>

void to_lower(char *string, int len_string) {
    for (int i = 0; i < len_string; ++i) {
        if (string[i] >= 'A' && string[i] <= 'Z') {
            string[i] += 32;
        }
    }
}

int main() {
    // Получаем доступ к разделяемой памяти и семафору
    sem_t *sem = sem_open("/semaphore", 0);
    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }

    // Чтение строки из разделяемой памяти
    int len_string;
    char *string = (char *)malloc(1024);  // Размер памяти для строки
    if (string == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    // Преобразуем строку в нижний регистр
    sem_wait(sem);
    int len = *((int *)string);
    to_lower(string, len);
    sem_post(sem);

    free(string);
    return 0;
}
