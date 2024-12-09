#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>  

typedef struct {
    int *arr;
    int low;
    int cnt;
    int dir;
    int thread_id; 
} SortArgs;

void bitonic_merge(int arr[], int low, int cnt, int dir) {
    if (cnt > 1) {
        int k = cnt / 2;
        for (int i = low; i < low + k; i++) {
            if (dir == (arr[i] > arr[i + k])) {
                int temp = arr[i];
                arr[i] = arr[i + k];
                arr[i + k] = temp;
            }
        }
        bitonic_merge(arr, low, k, dir);
        bitonic_merge(arr, low + k, k, dir);
    }
}

void bitonic_sort(int arr[], int low, int cnt, int dir) {
    if (cnt > 1) {
        int k = cnt / 2;
        bitonic_sort(arr, low, k, 1);  
        bitonic_sort(arr, low + k, k, 0); 
        bitonic_merge(arr, low, cnt, dir);
    }
}


void* bitonic_sort_thread(void *args) {
    SortArgs *sort_args = (SortArgs*)args;
    //printf("Thread %d started\n", sort_args->thread_id);  // Выводим информацию о запуске потока

    bitonic_sort(sort_args->arr, sort_args->low, sort_args->cnt, sort_args->dir);

    //printf("Thread %d finished\n", sort_args->thread_id);  // Выводим информацию о завершении потока
    return NULL;
}

// Функция для записи массива в файл
void write_to_file(int arr[], int n, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }
   
    fprintf(file, "Result: ");
    for (int i = 0; i < n; i++) {
        fprintf(file, "%d ", arr[i]);
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_threads>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        fprintf(stderr, "Invalid number of threads\n");
        return 1;
    }

    int n = 1000000;  
    int *arr = (int*)malloc(n * sizeof(int));

    if (arr == NULL) {
        perror("Memory allocation failed");
        return 1;
    }

    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 1000;
    }

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    pthread_t *threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    SortArgs *sort_args = (SortArgs*)malloc(num_threads * sizeof(SortArgs));

    int chunk_size = n / num_threads;

    for (int i = 0; i < num_threads; i++) {
        sort_args[i].arr = arr;
        sort_args[i].low = i * chunk_size;
        sort_args[i].cnt = (i == num_threads - 1) ? n - (i * chunk_size) : chunk_size;
        sort_args[i].dir = 1;  
        sort_args[i].thread_id = i;  

        pthread_create(&threads[i], NULL, bitonic_sort_thread, &sort_args[i]);
    }

   
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    
    bitonic_merge(arr, 0, n, 1);
    gettimeofday(&end_time, NULL);

    double time_taken = (end_time.tv_sec - start_time.tv_sec) + 
                        (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

    
    write_to_file(arr, n, "output.txt");

    
    printf("Time taken for sorting: %.6f seconds\n", time_taken);

    free(arr);
    free(threads);
    free(sort_args);

    return 0;
}
