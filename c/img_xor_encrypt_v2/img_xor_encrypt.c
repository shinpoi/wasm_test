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
        sum += raw_arr[i];
        fill_arr[i] = RL(raw_arr[i], (sum % 8));
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

void gen_block(int* l, int x, int y, int w, int size) {
    int n = 0;
    for (int i=0; i<size; i++) {
        for (int j=0; j<size; j++) {
            l[n] = (y+i)*w + x + j*4;
            n += 1;
        }
    }
}

uint8_t key[GAP] = {0};
int key_sum = 0;
int key_i = 0;
void init_key() {
    int key_sum = 0;
    int key_i = 0;
    for (int i=0; i<GAP; i++) {
        key[i] = 0;
    }
}

uint8_t get_key() {
    key_sum += key[key_i];

    key[key_i] = xor4(key[key_i], key[key_i+1], key[key_i+2], key[key_i+3]);
    key[key_i] = RL(key[key_i], (key_sum % 8));

    uint8_t res = key[key_i];
    key_i += 1;
    if (key_i > GAP - 4) {
        key_i = 0;
    }
    return res;
}

void mask(uint8_t *key_raw, int key_size, uint8_t *img, int img_size, int h_px, int w_px) {
    if (key_size < 4) {
        return;
    }

    init_key();
    fill(key_raw, key_size, key);

    // int offset = 0;
    int i = 0;
    int sum = 0;

    int size = 5;
    int w_size = size * 4;
    h_px -= h_px % size;
    w_px -= w_px % w_size;
    int w_bit = w_px*4;

    for (int y=0; y<h_px; y+=size) {
        for (int x=0; x<w_bit; x+=w_size) {
            int l[size*size];
            uint8_t k_bit = get_key();
            gen_block(l, x, y, w_bit, size);
            for (int j=0; j<size*size; j++) {
                img[l[j]] = img[l[j]] ^ k_bit;
                img[l[j]+1] = img[l[j]+1] ^ k_bit;
                img[l[j]+2] = img[l[j]+2] ^ k_bit;
            }
        }
    }
}
