#include<stdlib.h>
#include<stdint.h>
#include<stdio.h>
#include<math.h>

#define MODE_RGBA 4
#define MODE_RGB  3
#define MODE_GRAY 1

#define PAD_NUM 0

#define CLAMP_UINT8(NUM)  round( (NUM > 255)? 255 : (NUM < 0)? 0 : NUM )
#define MAX(a, b)  ((a)>(b)? (a): (b))
#define MIN(a, b)  ((a)<(b)? (a): (b))

/*--------------------- type define ---------------------*/
typedef struct wasm_img {
   int w;
   int h;
   int len;
   int mode; // rgba=4, gray=1
   uint8_t* data;
} wasm_img;

typedef struct filter2d {
    int w;
    int h;
    float* data;  // float[h][w]
} filter2d;

/*------------------- static func ---------------------*/
static inline int IMG_INDEX(int y, int x, wasm_img img) {
    return img.mode * (y*img.w + x);
}

static inline int GET_PX(int y, int x, wasm_img img, int channel) {
    if ((y >= img.h) || (x >= img.w) || (y < 0) || (x < 0)) {
        return PAD_NUM;
    } else {
        return img.data[IMG_INDEX(y, x, img) + channel];
    }
}

static inline int GET_PX(int y, int x, wasm_img img) {
    return GET_PX(y, x, img, 0);
}

static inline void SET_PX(int y, int x, wasm_img img, uint8_t value) {
    img.data[IMG_INDEX(y, x, img)] = value;
}

/*---------------------- main func ---------------------*/
wasm_img rgba_2_gray(wasm_img src, uint8_t* dst_arr);
wasm_img apply_filter2d(wasm_img src, uint8_t* dst_arr, filter2d filter, int mode, int channel);
wasm_img apply_filter2d(wasm_img src, uint8_t* dst_arr, filter2d filter);
wasm_img max_pooling(wasm_img src, uint8_t* dst_arr, int pool_size, int stride);


