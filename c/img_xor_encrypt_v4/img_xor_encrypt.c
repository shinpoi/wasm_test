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
ciper: Ciper_W * Ciper_H array;  ciper[before_i] = after_i
mode: encrypt:1, decrypt: -1
*/
void init_exchange_mapper(uint8_t *key, int key_size, int *ciper) {
    int RCi = 0;
    int RCj = 0;
    int j = 0;

    for (int i=0; i< Ciper_W * Ciper_H; i++) {
        ciper[i] = i;
    }
    for (int i=0; i< Ciper_W * Ciper_H; i++) {
        j = (j + ciper[i] + key[i % key_size]) % (Ciper_W * Ciper_H);
        int t = ciper[i];
        ciper[i] = ciper[j];
        ciper[j] = t;
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
    // printf("[debug] key: %s, key_size: %d, h_px: %d, w_px: %d, mode: %d\n", key, key_size, h_px, w_px, mode);
    int ciper[Ciper_W * Ciper_H];

    // need at least 4 byte string as key
    if (key_size < 4) {
        return;
    }

    init_exchange_mapper(key, key_size, ciper);
    int step_h = h_px/Ciper_H;
    int step_w = w_px/Ciper_W;
    // printf("[debug] step_h: %d, step_w: %d\n", step_h, step_w);

    // for block { for piexl_in_block { exchange } }
    // encrypt: 0 -> max;  decrypt: max -> 0
    int H_offset = mode > 0 ? 0: Ciper_H-1;
    int W_offset = mode > 0 ? 0: Ciper_W-1;
    for (int by=0 + H_offset; by<Ciper_H && by>=0; by+=mode) {
        for (int bx=0 + W_offset; bx<Ciper_W && bx>=0; bx+=mode) {
            int ciper_i = by*Ciper_W + bx;
            int target_i = ciper[ciper_i];
            int ty = target_i / Ciper_W;
            int tx = target_i % Ciper_W;
            for (int h=0; h<step_h; h++) {
                for (int w=0; w<step_w; w++) {
                    int i = ( (h + by*step_h)*w_px + (w + bx*step_w) )*4;
                    int t = ( (h + ty*step_h)*w_px + (w + tx*step_w) )*4;
                    uint8_t r, g, b;
                    r = img[i];
                    g = img[i+1];
                    b = img[i+2];

                    img[i] = img[t];
                    img[i+1] = img[t+1];
                    img[i+2] = img[t+2];

                    img[t] = r;
                    img[t+1] = g;
                    img[t+2] = b;

                }
            }
        }
    }
}


