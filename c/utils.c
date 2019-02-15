#include<stdlib.h>
#include<stdint.h>
#include<stdio.h>
#include<math.h>

#define MODE_RGBA 4
#define MODE_RGB  3
#define MODE_GRAY 1

#define PAD_NUM 0

#define CLAMP_UINT8(NUM)  ( (NUM > 255)? 255 : (NUM < 0)? 0 : NUM )

/*---------------------- utils.utils ---------------------*/
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

static inline int IMG_INDEX(int y, int x, wasm_img img) {
    return img.mode * (y*img.w + x);
}

static inline int GET_PX(int y, int x, wasm_img img) {
    if ((y >= img.h) || (x >= img.w) || (y < 0) || (x < 0)) {
        return PAD_NUM;
    } else {
        return img.data[IMG_INDEX(y, x, img)];
    }
}

static inline void SET_PX(int y, int x, wasm_img img, uint8_t value) {
    img.data[IMG_INDEX(y, x, img)] = value;
}

/*---------------------- main func ---------------------*/
wasm_img rgba_2_gray(wasm_img src, uint8_t* dst_arr) {
    if (src.mode == MODE_GRAY) return src;

    wasm_img dst = {src.w, src.h, src.len/MODE_RGBA, MODE_GRAY, dst_arr};
    for (int i=0; i<src.len; i+=4) {
        // dst.data[i/4] = (int)(0.2126*src.data[i] + 0.7152*src.data[i+1] + 0.0722*src.data[i+2]);  // ITU BT.709
        dst.data[i/4] = (int)round(0.299*src.data[i] + 0.587*src.data[i+1] + 0.114*src.data[i+2]);  // ITU BT.601
    }
    return dst;
}

wasm_img apply_filter2d(wasm_img src, uint8_t* dst_arr, filter2d filter) {
    // TODO: support rgba (ch<0:all , ch=0/1/2: r/g/b and return gray-mode) 
    // only for MODE_GRAY & w, h is even:
    int fw = (filter.w - 1)/2;
    int fh = (filter.h - 1)/2;
    wasm_img dst = {src.w, src.h, src.len, src.mode, dst_arr};

    int i_px = 0; 
    for (int y=0; y<src.h; y++) {
        for (int x=0; x<src.w; x+=src.mode) {
            float res = 0;
            // TODO: condition: out of bounds？ or support pad?
            for (int fy=0; fy<filter.h; fy++) {
                for (int fx=0; fx<filter.w; fx++) {
                    res += filter.data[fy*filter.w + fx] * GET_PX(y+fy-fh, x+fx-fw, src);
                }
            }
            // SET_PX(y, x, dst, (int)round(CLAMP_UINT8(res)));
            dst.data[i_px] = (int)round(CLAMP_UINT8(res));
            i_px += dst.mode;
        }
    }
    return dst;
}

wasm_img max_pooling(wasm_img src, uint8_t* dst_arr, int pool_size) {
    wasm_img dst = {src.w/pool_size, src.h/pool_size, src.len/(pool_size*pool_size), src.mode, dst_arr};
    // TODO: pooling
    return dst;
}




/*---------------------- test ---------------------*/
void wasm_img_eq(wasm_img exc, wasm_img act, char* desc) {
    printf("[%s] wasm_img_eq: start\n", desc);
    if ((exc.w != act.w) || (exc.h != act.h) || (exc.len != act.len) || (exc.mode != act.mode)) {
        printf("except: {w: %d, h: %d, len: %d, mode: %d}\n", exc.w, exc.h, exc.len, exc.mode);
        printf("actual: {w: %d, h: %d, len: %d, mode: %d}\n", act.w, act.h, act.len, act.mode);
        return;
    }
    for (int i=0; i<exc.len; i++) {
        if (exc.data[i] != act.data[i]) printf("data[%d]: %d != %d\n", i, exc.data[i], act.data[i]);
    }
    printf("[%s] wasm_img_eq: fin\n", desc);
}

void assert_true(int r, char* msg) {
    if (!r) printf("> %s: False!\n", msg);
}

void test_macros() {
    printf("[test_macros] start\n");
    uint8_t test_data[6] = {11,12,13, 21,22,23};
    wasm_img sample = {3, 2, 3*2, MODE_GRAY, test_data};
    assert_true(IMG_INDEX(1, 1, sample) == (1*3 + 1), "IMG_INDEX(Y, X, IMG)");

    assert_true(CLAMP_UINT8(256) == 255, "CLAMP_UINT8(NUM) >255");
    assert_true(CLAMP_UINT8(-1) == 0, "CLAMP_UINT8(NUM) <0");
    assert_true(CLAMP_UINT8(128) == 128, "CLAMP_UINT8(NUM) normal");

    assert_true(GET_PX(1, 1, sample) == 22, "GET_PX(Y, X, IMG) normal");
    assert_true(GET_PX(-1, 1, sample) == PAD_NUM, "GET_PX(Y, X, IMG) y<0");
    assert_true(GET_PX(1, -1, sample) == PAD_NUM, "GET_PX(Y, X, IMG) x<0");
    assert_true(GET_PX(3, 1, sample) == PAD_NUM, "GET_PX(Y, X, IMG) y>h");
    assert_true(GET_PX(1, 4, sample) == PAD_NUM, "GET_PX(Y, X, IMG) x>w");

    SET_PX(1, 1, sample, 100);
    assert_true(GET_PX(1, 1, sample) == 100, "SET_PX(Y, X, IMG, V)");
    printf("[test_macros] end\n");
}

void test_rgba_2_gray() {
    uint8_t test_data[4*9] = {
        0,0,0,255, 10,10,10,255, 20,20,20,255,
        100,100,100,255, 110,110,110,255, 120,120,120,255,
        200,200,200,255, 210,210,210,255, 220,220,220,255
    };
    uint8_t test_dst_data[9] = {0};

    uint8_t except_dst_data[9] = {0,10,20, 100,110,120, 200,210,220};
    wasm_img except_dst = {3, 3, 3*3, MODE_GRAY, except_dst_data};

    wasm_img test_src = {3, 3, 3*3*MODE_RGBA, MODE_RGBA, test_data};
    wasm_img test_dst = rgba_2_gray(test_src, test_dst_data);

    wasm_img_eq(except_dst, test_dst, "test_rgba_2_gray");
}

/*
0   10  20        0   0   0
100 110 120  -->  90  0   130
200 210 220       255 255 255
*/
void test_apply_filter2d() {
    uint8_t test_data[4*9] = {0,10,20, 100,110,120, 200,210,220};
    uint8_t dst_data[9] = {0};
    wasm_img test_src = {3, 3, 3*3*MODE_GRAY, MODE_GRAY, test_data};

    float test_filter_data[9] = {0,-1,0, -1,4,-1, 0,-1,0};
    filter2d lapcian = {3, 3, test_filter_data};

    uint8_t except_dst_data[9] = {0,0,0, 90,0,130, 255,255,255};
    wasm_img except_dst = {3, 3, 3*3, MODE_GRAY, except_dst_data};

    wasm_img test_dst = apply_filter2d(test_src, dst_data, lapcian);
    wasm_img_eq(except_dst, test_dst, "test_apply_filter2d");
}


int main(int argc, char* argv[]) {
    printf("compile success!\n");
    // TODO: test function:
    test_macros();
    test_rgba_2_gray();
    test_apply_filter2d();

    return 0;
}
