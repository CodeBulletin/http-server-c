#include "map.h"
#include <stdio.h>

uint32_t murmurhash3(const char *key, size_t len, uint32_t seed) {
    uint32_t c1 = 0xcc9e2d51;
    uint32_t c2 = 0x1b873593;
    uint32_t r1 = 15;
    uint32_t r2 = 13;
    uint32_t m = 5;
    uint32_t n = 0xe6546b64;

    uint32_t hash = seed;

    const int nblocks = len / 4;
    const uint32_t *blocks = (const uint32_t *)(key + nblocks * 4);
    int i;
    for (i = -nblocks; i; i++) {
        uint32_t k = blocks[i];

        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;

        hash ^= k;
        hash = (hash << r2) | (hash >> (32 - r2));
        hash = hash * m + n;
    }

    const uint8_t *tail = (const uint8_t *)(key + nblocks * 4);
    uint32_t k1 = 0;

    switch (len & 3) {
        case 3:
            k1 ^= tail[2] << 16;
        case 2:
            k1 ^= tail[1] << 8;
        case 1:
            k1 ^= tail[0];
            k1 *= c1;
            k1 = (k1 << r1) | (k1 >> (32 - r1));
            k1 *= c2;
            hash ^= k1;
    }

    hash ^= len;
    hash ^= (hash >> 16);
    hash *= 0x85ebca6b;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35;
    hash ^= (hash >> 16);

    return hash;
}

size_t hash(uint8_t *key) {
    return murmurhash3((char *)key, strlen((char *)key), 0);
}

void resize_hashmap(struct hashmap *hashmap) {
    int old_capacity = hashmap->capacity;
    hashmap->capacity *= 2;
    struct map **old_buckets = hashmap->maps;
    hashmap->maps = (struct map **) malloc(hashmap->capacity * sizeof(struct map *));

    for (int i = 0; i < hashmap->capacity; i++) {
        hashmap->maps[i] = NULL;
    }

    for (int i = 0; i < old_capacity; i++) {
        struct map *entry = old_buckets[i];
        while (entry != NULL) {
            insert(hashmap, entry->key, entry->value);
            struct map *previous = entry;
            entry = entry->next;
            free(previous);
        }
    }
    
    free(old_buckets);
}

struct hashmap *create_hashmap(size_t capacity) {
    struct hashmap *hashmap = (struct hashmap *) malloc(sizeof(struct hashmap));
    hashmap->maps = (struct map **) malloc(capacity * sizeof(struct map *));
    for (int i = 0; i < capacity; i++) {
        hashmap->maps[i] = NULL;
    }
    hashmap->size = 0;
    hashmap->capacity = capacity;
    return hashmap;
}

void insert(struct hashmap *hashmap, uint8_t *key, uint8_t *value)  {
    if (hashmap->size >= hashmap->capacity) {
        resize_hashmap(hashmap);
    }
    
    struct map *entry = (struct map *) malloc(sizeof(struct map));
    entry->key = malloc(strlen((char *)key) + 1);
    strncpy((char *)entry->key, (char *)key, strlen((char *)key) + 1);
    entry->value = malloc(strlen((char *)value) + 1);
    strncpy((char *)entry->value, (char *)value, strlen((char *)value) + 1);
    entry->next = NULL;
    
    unsigned int index = hash(key) % hashmap->capacity;
    if (hashmap->maps[index] == NULL) {
        hashmap->maps[index] = entry;
        hashmap->size++;
        return;
    }
    entry->next = hashmap->maps[index];
    hashmap->maps[index] = entry;
    hashmap->size++;
}

uint8_t *get(struct hashmap *hashmap, uint8_t *key) {
    unsigned int index = hash(key) % hashmap->capacity;
    struct map *entry = hashmap->maps[index];
    
    while (entry) {
        if (strcmp((char *)entry->key, (char *)key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }
    
    return NULL;
}

void free_hashmap(struct hashmap *hashmap) {
    for (int i = 0; i < hashmap->capacity; i++) {
        struct map *entry = hashmap->maps[i];
        while (entry != NULL) {
            struct map *prev = entry;
            entry = entry->next;
            free(prev->key);
            free(prev->value);
            free(prev);
        }
    }
    free(hashmap->maps);
    free(hashmap);
}