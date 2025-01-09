#include "library.h"

typedef struct Allocator {
    void *memory;
    size_t size;
    uint8_t *bitmap;
    size_t block_size;
} Allocator;

EXPORT Allocator* allocator_create(void *const memory, const size_t size) {
    if (!memory || size == 0) return NULL;

    Allocator *allocator = (Allocator *)memory;
    allocator->memory = (void *)((uint8_t *)memory + sizeof(Allocator));
    allocator->size = size - sizeof(Allocator);
    allocator->block_size = 32;
    allocator->bitmap = (uint8_t *)allocator->memory;

    size_t bitmap_size = allocator->size / allocator->block_size / 8;
    memset(allocator->bitmap, 0, bitmap_size); // Все блоки свободны
    allocator->memory = (uint8_t *)allocator->bitmap + bitmap_size;

    return allocator;
}

EXPORT void allocator_destroy(Allocator *const allocator) {
    if (allocator) {
        memset(allocator, 0, allocator->size);
    }
}

EXPORT void* allocator_alloc(Allocator *const allocator, const size_t size) {
    if (!allocator || size == 0 || size > allocator->size) return NULL;

    size_t blocks_needed = (size + allocator->block_size - 1) / allocator->block_size;
    size_t total_blocks = allocator->size / allocator->block_size;
    uint8_t *bitmap = allocator->bitmap;

    size_t free_blocks = 0;
    for (size_t i = 0; i < total_blocks; ++i) {
        if (!(bitmap[i / 8] & (1 << (i % 8)))) {
            ++free_blocks;
            if (free_blocks == blocks_needed) {
                size_t start_block = i - blocks_needed + 1;
                for (size_t j = start_block; j <= i; ++j) {
                    bitmap[j / 8] |= (1 << (j % 8));  // Помечаем блоки как занятые
                }
                return (uint8_t *)allocator->memory + start_block * allocator->block_size;
            }
        } else {
            free_blocks = 0;
        }
    }

    // Если не найдено достаточное количество свободных блоков, пытаемся выполнить деление:
    size_t block_size = allocator->block_size;
    while (block_size > 1) {  // Продолжаем делить блоки, пока не дойдем до минимального размера
        block_size /= 2;
        free_blocks = 0;

        for (size_t i = 0; i < total_blocks; ++i) {
            if (!(bitmap[i / 8] & (1 << (i % 8)))) {
                ++free_blocks;
                if (free_blocks == blocks_needed) {
                    size_t start_block = i - blocks_needed + 1;
                    for (size_t j = start_block; j <= i; ++j) {
                        bitmap[j / 8] |= (1 << (j % 8));  // Помечаем блоки как занятые
                    }
                    return (uint8_t *)allocator->memory + start_block * allocator->block_size;
                }
            } else {
                free_blocks = 0;
            }
        }
    }

    // Если не удалось выделить память, возвращаем NULL
    return NULL;
}

EXPORT void allocator_free(Allocator *const allocator, void *const memory) {
    if (!allocator || !memory) return;

    size_t offset = (uint8_t *)memory - (uint8_t *)allocator->memory;
    if (offset % allocator->block_size != 0) return;

    size_t block_index = offset / allocator->block_size;

    // Помечаем блок как свободный в битовой карте
    allocator->bitmap[block_index / 8] &= ~(1 << (block_index % 8));

    // Начинаем проверку для объединения "двойников"
    size_t current_block_size = allocator->block_size;

    // Пока блок можно "объединять", проверяем его двойника
    while (current_block_size < allocator->size) {
        size_t buddy_index = block_index ^ (current_block_size / 2);  // Индекс двойника
        if (buddy_index < allocator->size / current_block_size && 
            !(allocator->bitmap[buddy_index / 8] & (1 << (buddy_index % 8)))) {
            // Если двойник свободен, объединяем блоки
            allocator->bitmap[block_index / 8] &= ~(1 << (block_index % 8));  
            allocator->bitmap[buddy_index / 8] &= ~(1 << (buddy_index % 8));  
            block_index = block_index / 2; // Переходим к родительскому блоку
            current_block_size *= 2;
        } else {
            break;
        }
    }
}
