#ifndef HELPER_H
#define HELPER_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

uint8_t* integer_to_sring(uint32_t num) {
    uint8_t *str = malloc(11);
    int i = 0;
    while (num > 0) {
        str[i] = num % 10 + '0';
        num /= 10;
        i++;
    }
    str[i] = '\0';
    int j = 0;
    i--;
    while (j < i) {
        uint8_t temp = str[j];
        str[j] = str[i];
        str[i] = temp;
        j++;
        i--;
    }
    return str;
}

uint8_t* recvall(int id, uint8_t *buffer, size_t buffer_size, size_t chunk_size) {
    size_t bytes_received = 0;
    while (bytes_received < buffer_size) {
        int bytes = read(id, buffer + bytes_received, chunk_size);
        if (bytes < 0) {
            perror("Error receiving data from client");
            return NULL;
        }
        bytes_received += bytes;
    }
    return buffer;
}

#endif