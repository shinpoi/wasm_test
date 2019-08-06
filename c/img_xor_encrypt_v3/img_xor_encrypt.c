//emcc img_xor_encrypt.c -std=c11 -Os -s EXPORTED_FUNCTIONS='["_mask", "_cr_buffer", "_fr_buffer"]' -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 -o xor_encrypt.js

#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>

#define Ciper_W 100
#define Ciper_H 100

uint8_t* cr_buffer(int size) {
    return (uint8_t*)malloc(size * sizeof(uint8_t));
}

void fr_buffer(uint8_t* p) {
    free(p);
}

/* init ciper to encrype image */
void init_ciper(uint8_t *key, int key_size, uint8_t *ciper) {
    uint8_t S[256] = {0}; 
    uint8_t RCi = 0;
    uint8_t RCj = 0;

    // RC4 - init S
    uint8_t j = 0;
    for (int i=0; i< 256; i++) {
        S[i] = i;
    }
    for (int i=0; i< 256; i++) {
        j = (j + S[i] + key[i % key_size]) % 256;
        uint8_t t = S[i];
        S[i] = S[j];
        S[j] = t;
    }

    // RC4 - next
    for (int i=0; i < Ciper_W; i++) {
        for (int j=0; j < Ciper_H; j++) {
            RCi = (RCi + 1) % 256;
            RCj = (RCj + S[RCi]) % 256;
            uint8_t t = S[RCi];
            S[RCi] = S[RCj];
            S[RCj] = t;
            ciper[i*Ciper_W + j] = S[(S[RCi] + S[RCj]) % 256];
        }
    }
}

/*
key:         key string
key_size:    length of key (byte)
img:         image data (rgba)
h_px & w_px: height & width of image
mode: encrypt: 1; decrypt: -1
*/
void mask(uint8_t *key, int key_size, uint8_t *img, int h_px, int w_px, int mode) {
    uint8_t ciper[Ciper_W * Ciper_H] = {0};

    // need at least 4 byte string as key
    if (key_size < 4) {
        return;
    }

    init_ciper(key, key_size, ciper);

    for (int h=0; h<h_px; h++) {
        for (int w=0; w<w_px; w++) {
            int i = (h*w_px + w)*4;
            int x = (h/(float)h_px)*Ciper_H;
            int y = (w/(float)w_px)*Ciper_W;
            uint8_t k = ciper[x*Ciper_W + y];
            img[i] += mode*k;
            img[i+1] += mode*k;
            img[i+2] += mode*k;
        }
    }
}
