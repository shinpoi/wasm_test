#include "../utils.h"
#include<iostream>
#include<string>
using namespace std;

/*------------------- test func ---------------------*/
void wasm_img_eq(wasm_img exc, wasm_img act, string desc);
void assert_true(int r, string msg);
void test_macros();
void test_rgba_2_gray();
void test_apply_filter2d();
void test_max_pooling();


/*---------------------- test ---------------------*/
void wasm_img_eq(wasm_img exc, wasm_img act, string desc) {
    if ((exc.w != act.w) || (exc.h != act.h) || (exc.len != act.len) || (exc.mode != act.mode)) {
        printf("except: {w: %d, h: %d, len: %d, mode: %d}\n", exc.w, exc.h, exc.len, exc.mode);
        printf("actual: {w: %d, h: %d, len: %d, mode: %d}\n", act.w, act.h, act.len, act.mode);
        return;
    }
    for (int i=0; i<exc.len; i++) {
        if (exc.data[i] != act.data[i]) printf("data[%d]: %d != %d\n", i, exc.data[i], act.data[i]);
    }
    printf("[%s] wasm_img_eq: ok\n", desc.c_str());
}

void assert_true(int r, string msg) {
    if (!r) printf("> %s: False!\n", msg.c_str());
}

void test_macros() {
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
    printf("[test_macros] ok\n");
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
    uint8_t test_data[9] = {0,10,20, 100,110,120, 200,210,220};
    wasm_img test_src = {3, 3, 3*3*MODE_GRAY, MODE_GRAY, test_data};

    float test_filter_data[9] = {0,-1,0, -1,4,-1, 0,-1,0};
    filter2d lapcian = {3, 3, test_filter_data};

    uint8_t except_dst_data[9] = {0,0,0, 90,0,130, 255,255,255};
    wasm_img except_dst = {3, 3, 3*3, MODE_GRAY, except_dst_data};

    uint8_t dst_data[9] = {0};
    wasm_img test_dst = apply_filter2d(test_src, dst_data, lapcian);
    wasm_img_eq(except_dst, test_dst, "test_apply_filter2d");
}

/*
0   10  20        110 120
100 110 120  -->  210 220
200 210 220
*/
void test_max_pooling() {
    uint8_t test_data[9] = {0,10,20, 100,110,120, 200,210,220};
    wasm_img test_src = {3, 3, 3*3*MODE_GRAY, MODE_GRAY, test_data};

    uint8_t except_dst_data[4] = {110,120, 210,220};
    wasm_img except_dst = {2, 2, 2*2, MODE_GRAY, except_dst_data};

    uint8_t dst_data[4] = {0};
    wasm_img test_dst = max_pooling(test_src, dst_data, 3, 2);
    wasm_img_eq(except_dst, test_dst, "test_max_pooling");
}

int main(int argc, char* argv[]) {
    // TODO: test function:
    test_macros();
    test_rgba_2_gray();
    test_apply_filter2d();
    test_max_pooling();

    return 0;
}
