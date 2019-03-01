#include "utils.h"
using namespace std;

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

wasm_img apply_filter2d(wasm_img src, uint8_t* dst_arr, filter2d filter, int mode, int channel) {
    // TODO: support rgba (ch<0:all , ch=0/1/2: r/g/b and return gray-mode) 
    // only for MODE_GRAY & w, h is even:
    int fw = (filter.w - 1)/2;
    int fh = (filter.h - 1)/2;
    wasm_img dst = {src.w, src.h, src.len, src.mode, dst_arr};

    int i_px = 0; 
    for (int y=0; y<dst.h; y++) {
        for (int x=0; x<dst.w; x+=dst.mode) {
            float res = 0;
            // TODO: condition: out of bounds？ or support pad?
            for (int fy=0; fy<filter.h; fy++) {
                for (int fx=0; fx<filter.w; fx++) {
                    res += filter.data[fy*filter.w + fx] * GET_PX(y+fy-fh, x+fx-fw, src);
                }
            }
            // SET_PX(y, x, dst, (int)round(CLAMP_UINT8(res)));
            dst.data[i_px] = CLAMP_UINT8(res);
            i_px += dst.mode;
        }
    }
    return dst;
}

wasm_img apply_filter2d(wasm_img src, uint8_t* dst_arr, filter2d filter) {
    return apply_filter2d(src, dst_arr, filter, src.mode, 0);
}

wasm_img max_pooling(wasm_img src, uint8_t* dst_arr, int pool_size, int stride, int channel) {
    wasm_img dst = {(src.w+1)/stride, (src.h+1)/stride, (int)((src.w+1)/stride) * (int)((src.h+1)/stride), src.mode, dst_arr};
    int fl = (pool_size - 1)/2;

    int i_px = 0; 
    for (int y=0; y<dst.h; y++) {
        for (int x=0; x<dst.w; x+=dst.mode) {
            int max = 0;
            for (int fy=0; fy<pool_size; fy++) {
                for (int fx=0; fx<pool_size; fx++) {
                    max = MAX(max, GET_PX(y*stride + fy-fl, x*stride + fx-fl, src, channel));
                }
            }
            // SET_PX(y, x, dst, (int)round(CLAMP_UINT8(res)));
            dst.data[i_px] = max;
            i_px += dst.mode;
        }
    }
    return dst;
}

wasm_img max_pooling(wasm_img src, uint8_t* dst_arr, int pool_size, int stride) {
    return max_pooling(src, dst_arr, pool_size, stride, 0);
}