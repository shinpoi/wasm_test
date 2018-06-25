//emcc filter.c -std=c11 -Os -o filter.js -s EXPORTED_FUNCTIONS='["_hist_match", "_cr_buffer", "_fr_buffer"]' -s ALLOW_MEMORY_GROWTH=1

#include<stdint.h>
#include<stdlib.h>

void hist_atch(uint8_t* srcBuffer, uint8_t* refBuffer, int srcLen, int refLen);
void histMap(uint8_t* buffer, int len, double* histMap_R, double* histMap_G, double* histMap_B);
void hist(uint8_t* arr, int len, double* histArr, int offset);
void hist2map(double* hist, int histLen);
void calc_map(double* srcMap, double* refMap, int* map, int len);
double sum_f(double* arr, int len);


uint8_t* cr_buffer(int size) {
	return malloc(size);
}

void fr_buffer(uint8_t* p) {
	free(p);
}

void hist_match(uint8_t* srcBuffer, uint8_t* refBuffer, int srcLen, int refLen) {
	double srcHist[3][256] = {0};
	double refHist[3][256] = {0};
	int map[3][256] = {0};

	// for RGB
	for (int ch=0; ch<3; ch++) {
		hist(srcBuffer, srcLen, srcHist[ch], ch);
		hist(refBuffer, refLen, refHist[ch], ch);

		hist2map(srcHist[ch], 256);
		hist2map(refHist[ch], 256);

		calc_map(srcHist[ch], refHist[ch], map[ch], 256);

		for (int i=0; i<srcLen; i+=4) {
			srcBuffer[i+ch] = map[ch][srcBuffer[i+ch]];
		}
	}
}

void hist(uint8_t* arr, int len, double* histArr, int offset) {
	for (int i=0; i<len; i+=4) {
		if (arr[i+3]){
			histArr[arr[i+offset]]++;
		}
	}
}

void hist2map(double* hist, int histLen) {
	double sum = sum_f(hist, histLen);
	for (int i=0; i<histLen; i++) {
		hist[i] /= sum;
	}

	sum = 0;
	for (int i=0; i<histLen; i++){
		sum += hist[i];
		hist[i] = sum;
	}
}

void calc_map(double* srcMap, double* refMap, int* map, int len) {
	for (int i=0; i<len; i++){
		double minDiff = 2;
		double diff = 0;
		int nextJ = 0;
		for (int j=nextJ; j<len; j++) {
			if (diff == srcMap[i] - refMap[j]) {
				continue;
			} else {
				diff = srcMap[i] - refMap[j];
				diff = (diff > 0)?diff:-diff;
			}

			if (diff < minDiff) {
				minDiff = diff;
				map[i] = j;
			}
		}
		nextJ = i?i-1:0;
	}
}


double sum_f(double* arr, int len) {
	double sum = 0;
	for (int i=0; i<len; i++){
		sum += arr[i];
	}
	return sum;
}
