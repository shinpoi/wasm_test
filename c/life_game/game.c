//emcc game.c -std=c11 -Os -s EXPORTED_FUNCTIONS='["_init_world", "_next", "_cr_buffer", "_fr_buffer"]' -s ALLOW_MEMORY_GROWTH=1 -o game.js

#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>

int W = 0;
int H = 0;
int H_px = 0;
int W_px = 0;
int s_block = 0;
uint8_t* Space_now = NULL;
uint8_t* Space_next = NULL;
uint8_t* Img = NULL;

uint8_t* cr_buffer(int size) {
    return (uint8_t*)malloc(size * sizeof(uint8_t));
}

void fr_buffer(uint8_t* p) {
    free(p);
}

void clear_world() {
    W = 0;
    H = 0;
    H_px = 0;
    W_px = 0;
    s_block = 0;
    if (Space_now == NULL || Space_next == NULL || Img == NULL) {
        printf("world is NULL.\n");
        return;
    }
    fr_buffer(Space_now);
    fr_buffer(Space_next);
    fr_buffer(Img);
    printf("world cleared\n");
}

void init_world(int w, int h, int block_size, uint8_t* space, uint8_t* img) {
    clear_world();
    if (w == 0 || h == 0 || block_size == 0) {
        return;
    }
    W = w;
    H = h;
    s_block = block_size;
    H_px = H * s_block;
    W_px = W * 4 * s_block;
    Space_now = space;
    Space_next = cr_buffer(W * H);
    Img = img;
    printf("world inited\n");
}

const int limit = 1;
uint8_t get_status(int x, int y) {
    uint8_t sum = 0;
    for (int i = -limit; i<limit+1; i++) {
        for (int j = -limit; j<limit+1; j++) {
            if (i == 0 && j == 0) {continue;}
            sum += Space_now[((y+i+H) % H)*W + ((x+j+W)%W)];
        }
    }
    return sum;
}

void draw() {
    for (int y=0; y<H_px; y+=1) {
        for (int x=0; x<W_px; x+=4) {
            Img[y*W_px + x] = Img[y*W_px + x + 1] = Img[y*W_px + x + 2] = 0x11 + Space_now[(y/s_block)*W + (x/4/s_block)]*0xea;
            Img[y*W_px + x + 3] = 255;
        }
    }
}

void next() {
    for (int y=0; y<H; y++) {
        for (int x=0; x<W; x++) {
            uint8_t status = get_status(x, y);
            int i = y*W + x;
            if (Space_now[i]) {
                if (status == 2 || status == 3){
                    Space_next[i] = 1;
                } else {
                    Space_next[i] = 0;
                }
            } else {
                if (status == 3) {
                    Space_next[i] = 1;
                } else {
                    Space_next[i] = 0;
                }
            }
        }
    }
    // exchange now and next
    uint8_t* t_ptr = Space_now;
    Space_now = Space_next;
    Space_next = t_ptr;

    draw();
}
/*
int main() {
    unsigned int a = 9%10;
    unsigned int b = 10%10;
    unsigned int c = -1%10;

    int x[10] = {0,1,2,3,4,5,6,7,8,9};
    printf("%d, %d, %d\n", a, b, c);
    printf("%d, %d, %d\n", x[a], x[b], x[c]);
}*/