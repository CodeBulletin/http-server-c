#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct map {
    uint8_t *key;
    uint8_t *value;
    struct map *next;
};

struct hashmap {
    struct map **maps;
    size_t size;
    size_t capacity;
};

struct hashmap *create_hashmap(size_t capacity);

void insert(struct hashmap *hashmap, uint8_t *key, uint8_t *value);

uint8_t *get(struct hashmap *hashmap, uint8_t *key);

void free_hashmap(struct hashmap *hashmap);