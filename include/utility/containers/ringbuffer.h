#ifndef _CHEESOS2_UTILITY_CONTAINERS_RINGBUFFER_H
#define _CHEESOS2_UTILITY_CONTAINERS_RINGBUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RINGBUFFER_SIZE 1 << 10
#define RINGBUFFER_MASK 0b1111111111

typedef struct {
    uint8_t data[RINGBUFFER_SIZE];
    size_t write;
    size_t read;
} ringbuffer;

void ringbuffer_init(volatile ringbuffer* rb);
size_t ringbuffer_length(volatile ringbuffer* rb);
void ringbuffer_read(volatile ringbuffer* rb, uint8_t* output, size_t length);
bool ringbuffer_put(volatile ringbuffer* rb, uint8_t data);
void ringbuffer_remove(volatile ringbuffer* rb, size_t length);

#endif