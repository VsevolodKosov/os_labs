#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>

#define SHM_NAME "/shared_memory"
#define SEM_NAME "/semaphore"

typedef enum{
    OK,
    MemoryError
}status_code;

// Функция для получения строки с клавиатуры
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
        (*len_string)++;                  // Увеличиваем длину строки

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
    const char *args[] = { "./child1", NULL };  // Путь к дочерним процессам

    // Шаг 1: Создаем разделяемую память
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0777);
    if (shm_fd == -1) {
        perror("shm_open failed");
        exit(EXIT_FAILURE);
    }

    ftruncate(shm_fd, sizeof(int) + 1024);  // Размер разделяемой памяти (длина строки + сама строка)

    // Отображаем память в адресное пространство процесса
    char *shared_memory = (char *)mmap(NULL, sizeof(int) + 1024, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    // Шаг 2: Создаем семафор для синхронизации
    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0777, 1);  // Инициализируем семафор значением 1
    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }

    // Шаг 3: Считываем строку с клавиатуры
    int len_string;
    char *string;
    if (get_string(&string, &len_string) == MemoryError) {
        const char message[] = "MemoryError: Failed to allocate memory";
        write(STDERR_FILENO, message, sizeof(message));
        exit(EXIT_FAILURE);
    }

    // Сохраняем длину строки в разделяемую память
    memcpy(shared_memory, &len_string, sizeof(int));
    memcpy(shared_memory + sizeof(int), string, len_string);
    free(string);  // Освобождаем память для строки

    // Шаг 4: Создаем дочерние процессы
    int pid1 = fork();
    if (pid1 == -1) {
        perror("fork failed for child1");
        exit(EXIT_FAILURE);
    }
    else if (pid1 == 0) {  // Первый дочерний процесс
        sem_wait(sem);  // Ожидаем доступа к разделяемой памяти

        // Извлекаем строку из разделяемой памяти
        int child1_len = *((int *)shared_memory);
        char *child1_string = shared_memory + sizeof(int);

        // Преобразуем строку в нижний регистр
        to_lower(child1_string, child1_len);

        // Записываем обратно в разделяемую память
        memcpy(shared_memory + sizeof(int), child1_string, child1_len);

        sem_post(sem);  // Освобождаем доступ к разделяемой памяти
        exit(EXIT_SUCCESS);
    }

    int pid2 = fork();
    if (pid2 == -1) {
        perror("fork failed for child2");
        exit(EXIT_FAILURE);
    }
    else if (pid2 == 0) {  // Второй дочерний процесс
        sem_wait(sem);  // Ожидаем доступа к разделяемой памяти

        // Извлекаем строку из разделяемой памяти
        int child2_len = *((int *)shared_memory);
        char *child2_string = shared_memory + sizeof(int);

        // Удаляем лишние пробелы
        remove_extra_spaces(child2_string, &child2_len);

        // Записываем обратно в разделяемую память
        memcpy(shared_memory + sizeof(int), child2_string, child2_len);
        *((int *)shared_memory) = child2_len;  // Обновляем длину строки

        sem_post(sem);  // Освобождаем доступ к разделяемой памяти
        exit(EXIT_SUCCESS);
    }

    // Родительский процесс
    wait(NULL);  // Ждем завершения дочерних процессов
    wait(NULL);

    // Извлекаем итоговую строку из разделяемой памяти
    int final_len = *((int *)shared_memory);
    char *final_string = shared_memory + sizeof(int);

    // Выводим результат
    write(STDOUT_FILENO, final_string, final_len);
    write(STDOUT_FILENO, "\n", 1);

    // Освобождаем ресурсы
    sem_close(sem);
    sem_unlink(SEM_NAME);
    munmap(shared_memory, sizeof(int) + 1024);
    shm_unlink(SHM_NAME);

    return 0;
}
