#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

// Функция для слияния битонических последовательностей
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

// Функция битонической сортировки
void bitonic_sort(int arr[], int low, int cnt, int dir) {
    if (cnt > 1) {
        int k = cnt / 2;
        bitonic_sort(arr, low, k, 1); // сортировка по возрастанию
        bitonic_sort(arr, low + k, k, 0); // сортировка по убыванию
        bitonic_merge(arr, low, cnt, dir);
    }
}

// Функция для записи массива в файл
void write_to_file(int arr[], int n, const char *filename) {
    // Открытие файла для записи
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        // Ошибка при открытии файла
        write(2, "Error opening file\n", 19);
        exit(1);
    }

    // Запись строки "Result: " в файл
    const char *result_header = "Result: ";
    write(fd, result_header, 8);  // 8 - длина строки "Result: "

    // Запись элементов массива в файл
    for (int i = 0; i < n; i++) {
        char buffer[20];
        int len = snprintf(buffer, sizeof(buffer), "%d ", arr[i]);
        write(fd, buffer, len);
    }

    // Закрытие файла
    close(fd);
}

int main() {
    // Исходный массив
    int arr[] = {3, 7, 2, 5, 8, 1, 6, 4};
    int n = sizeof(arr) / sizeof(arr[0]);

    // Сортировка массива
    bitonic_sort(arr, 0, n, 1);

    // Запись отсортированного массива в файл
    write_to_file(arr, n, "output.txt");

    return 0;
}
