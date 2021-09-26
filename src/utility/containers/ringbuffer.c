#include "utility/containers/ringbuffer.h"

void ringbuffer_init(volatile ringbuffer* rb){
    rb->write = 0;
    rb->read = 0;
}

size_t ringbuffer_length(volatile ringbuffer* rb){
    return rb->write - rb->read;
}

void ringbuffer_read(volatile ringbuffer* rb, uint8_t* output, size_t length){
    for(size_t i = 0;i < length;++i){
        if(rb->write - rb->read > 0){
            output[i] = rb->data[rb->read & RINGBUFFER_MASK];
            ++rb->read;
        }else{
            output[i] = 0;
        }
    }
}

bool ringbuffer_put(volatile ringbuffer* rb, uint8_t data){
    if(rb->write - rb->read < RINGBUFFER_SIZE){
        rb->data[rb->write & RINGBUFFER_MASK] = data;
        ++rb->write;
        return true;
    }
    return false;
}

void ringbuffer_remove(volatile ringbuffer* rb, size_t length){
    for(size_t i = 0;i < length;++i){
        if(rb->write > rb->read) --rb->write;
    }
}