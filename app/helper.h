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