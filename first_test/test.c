//emcc test.c -std=c11 -Os -o test.js -s EXPORTED_FUNCTIONS='["_histMatch", "_cr_buffer", "_fr_buffer"]' -s ALLOW_MEMORY_GROWTH=1

#include<stdint.h>
#include<stdlib.h>

void histMap(uint8_t* buffer, unsigned int len, double* histMap_R, double* histMap_G, double* histMap_B);
void matchMap(double* histMap_src, double* histMap_ref, uint8_t* map);
double sum(double* array, unsigned int len);

unsigned char* cr_buffer(unsigned int size) {
	return malloc(size);
}

void fr_buffer(unsigned char* p) {
	free(p);
}

void histMatch(uint8_t* buffer_src, uint8_t* buffer_ref, unsigned int len_src, unsigned int len_ref) {
	double src_histMap_R[256] = {0};
	double src_histMap_G[256] = {0};
	double src_histMap_B[256] = {0};
	double ref_histMap_R[256] = {0};
	double ref_histMap_G[256] = {0};
	double ref_histMap_B[256] = {0};

	histMap(buffer_src, len_src, src_histMap_R, src_histMap_G, src_histMap_B);
	histMap(buffer_ref, len_ref, ref_histMap_R, ref_histMap_G, ref_histMap_B);

	uint8_t map_R[256] = {0};
	uint8_t map_G[256] = {0};
	uint8_t map_B[256] = {0};

	matchMap(src_histMap_R, ref_histMap_R, map_R);
	matchMap(src_histMap_G, ref_histMap_G, map_G);
	matchMap(src_histMap_B, ref_histMap_B, map_B);

	for (int i=0; i<len_src; i+=4) {
		buffer_src[i] = map_R[buffer_src[i]];
		buffer_src[i+1] = map_G[buffer_src[i+1]];
		buffer_src[i+2] = map_B[buffer_src[i+2]];
	}
}


// double hist_R/G/B[256] = {0}
void histMap(uint8_t* buffer, unsigned int len, double* histMap_R, double* histMap_G, double* histMap_B) {
	double hist_R[256] = {0};
	double hist_G[256] = {0};
	double hist_B[256] = {0};
		
	for(int i=0; i<len; i+=4) {
		if (buffer[i+3]){
			hist_R[buffer[i]] += 1;
			hist_G[buffer[i+1]] += 1;
			hist_B[buffer[i+2]] += 1;
		}
	}

	double sum_R = sum(hist_R, 256);
	double sum_G = sum(hist_G, 256);
	double sum_B = sum(hist_B, 256);

	for (int i=0; i<256; i++) {
		hist_R[i] = hist_R[i]/sum_R;
		hist_G[i] = hist_G[i]/sum_G;
		hist_B[i] = hist_B[i]/sum_B;
	}

	sum_R = sum_G = sum_B = 0;
	for (int i=0; i<256; i++) {
		sum_R += hist_R[i];
		sum_G += hist_G[i];
		sum_B += hist_B[i];
		histMap_R[i] = sum_R;
		histMap_G[i] = sum_G;
		histMap_B[i] = sum_B;
	}
}

// unsigned int map[256] = {0}
void matchMap(double* histMap_src, double* histMap_ref, uint8_t* map) {
	for (int i=0; i<256; i++){
		double min_diff = 2;
		double diff = 0;
		for (int j=map[i-1]; j<256; j++) {
			if (diff == histMap_src[i] - histMap_ref[j]) {
				continue;
			} else {
				diff = histMap_src[i] - histMap_ref[j];
				diff = (diff > 0)?diff:-diff;
			}

			if (diff < min_diff) {
				min_diff = diff;
				map[i] = j;
			}
		}
	}
}

double sum(double* array, unsigned int len) {
	double summ = 0;
	for (int i=0; i<len; i++){
		summ += array[i];
	}
	return summ;
}
