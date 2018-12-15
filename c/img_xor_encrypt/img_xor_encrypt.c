//emcc img_xor_encrypt.c -std=c11 -Os -s EXPORTED_FUNCTIONS='["_mask", "_cr_buffer", "_fr_buffer"]' -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1  -o xor_encrypt.js

#include<stdint.h>
#include<stdlib.h>

#define GAP 64
#define RL(x, n) ((x << n) | (x >> (8 - n)))

uint8_t* cr_buffer(int size) {
    return (uint8_t*)malloc(size * sizeof(uint8_t));
}

void fr_buffer(uint8_t* p) {
    free(p);
}

uint8_t xor4(uint8_t n1, uint8_t n2, uint8_t n3, uint8_t n4) {
    return RL(n1, (n3 % 8)) ^ RL(n2, (n4 % 8));
}

void fill(uint8_t *raw_arr, int l_raw, uint8_t *fill_arr) {
    if (l_raw > GAP) {
        for (int i=0; i<GAP; i++) {
            fill_arr[i] = raw_arr[i];
        }
        return;
    }

    int offset = 0;
    int sum = 0;
    for (int i=0; i<l_raw - 3; i++) {
        sum += fill_arr[i];
        fill_arr[i] = RL(fill_arr[i], (sum % 8));
        fill_arr[i] = xor4(raw_arr[i], raw_arr[i+1], raw_arr[i+2], raw_arr[i+3]);
    }
    fill_arr[l_raw-3] = xor4(raw_arr[l_raw-3], raw_arr[l_raw-2], raw_arr[l_raw-1], raw_arr[0]);
    fill_arr[l_raw-2] = xor4(raw_arr[l_raw-2], raw_arr[l_raw-1], raw_arr[0], raw_arr[1]);
    fill_arr[l_raw-1] = xor4(raw_arr[l_raw-1], raw_arr[0], raw_arr[1], raw_arr[2]);
    offset += l_raw;

    while (offset < GAP) {
        sum += fill_arr[offset];
        fill_arr[offset] = RL(fill_arr[offset], (sum % 8));
        fill_arr[offset] = xor4(fill_arr[offset], fill_arr[offset-1], fill_arr[offset-2], fill_arr[offset-3]);
        offset += 1;
    }
}

void mask(uint8_t *key_raw, int key_size, uint8_t *img, int img_size) {
    if (key_size < 4) {
        return;
    }

    uint8_t key[GAP] = {0};
    fill(key_raw, key_size, key);

    int offset = 0;
    int i = 0;
    int sum = 0;
    while (offset < img_size) {
        if ((offset + 1) % 4 != 0) {
            img[offset] = img[offset] ^ key[i];
            i += 1;
        }
        offset += 1;
        sum += key[i];
        key[i] = xor4(key[i], key[i+1], key[i+2], key[i+3]);
        key[i] = RL(key[i], (sum % 8));
        if (i > GAP - 4) {
            i = 0;
        }
    }
}
