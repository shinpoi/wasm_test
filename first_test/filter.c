//emcc filter.c -std=c11 -Os -o filter.js -s EXPORTED_FUNCTIONS='["_hist_match", "_rgb_exchange", "_cr_buffer", "_fr_buffer"]' -s ALLOW_MEMORY_GROWTH=1

#include<stdint.h>
#include<stdlib.h>

void hist_match(uint8_t* srcBuffer, uint8_t* refBuffer, int srcLen, int refLen);
void hist(uint8_t* arr, int len, double* histArr, int offset);
void hist2map(double* hist, int histLen);
void calc_map(double* srcMap, double* refMap, int* map, int len);
double sum_f(double* arr, int len);

// srcSize = refSize, scale image by JS.
void rgb_exchange(uint8_t* srcBuffer, uint8_t* refBuffer, int len);
void sort_with(uint8_t** pArr, int* indexArr, int len);
void restore(uint8_t** pArr, int* indexArr, int len);

uint8_t* cr_buffer(int size) {
	return malloc(size);
}

void fr_buffer(uint8_t* p) {
	free(p);
}

void rgb_exchange(uint8_t* srcBuffer, uint8_t* refBuffer, int len) {
	// init
	int l = len/4;
	uint8_t* pSrcBuffer[3][l];
	uint8_t* pRefBuffer[3][l];
	int indexArrs[3][l];

	for (int i=0; i<l; i++) {
		for (int ch=0; ch<3; ch++) {
			pSrcBuffer[ch][i] = srcBuffer + i*4 + ch;
			pRefBuffer[ch][i] = refBuffer + i*4 + ch;
		}
	}

	for (int i=0; i<len; i++) {
		for (int ch=0; ch<3; ch++) {
			indexArrs[ch][i] = i;
		}
	}

	// run
	for (int ch=0; ch<3; ch++) {
		// sort srcArr(with index) and refArr.
		sort_with(pSrcBuffer[ch], indexArrs[ch], l);
		sort_with(pRefBuffer[ch], NULL, l);

		// exchange sorted srcArr and refArr
		for (int i=0; i<l; i++) {
			*pSrcBuffer[ch][i] = *pRefBuffer[ch][i];
		}

		// restore srcArr by index
		restore(pSrcBuffer[ch], indexArrs[ch], l);
	}
}

void restore(uint8_t** pArr, int* indexArr, int len) {
	uint8_t t = 0;
	for (int i=0; i<len; i++) {
		t = *pArr[i];
		*pArr[i] = *pArr[indexArr[i]];
		*pArr[indexArr[i]] = t;
	}
}

void sort_with(uint8_t** pArr, int* indexArr, int len) {
	int index = 0;
	uint8_t min = 0;
	uint8_t b_min = 0;
	uint8_t t = 0;
	for (int i; i<len; i++) {
		b_min = min;
		min = 255;

		for (int j=0; j<len-i; j++) {
			if (*pArr[i] == b_min) { continue; }
			if (*pArr[j] < min) {
				min = *pArr[j];
				index = j;
			}
		}

		t = *pArr[index];
		*pArr[index] = *pArr[i];
		*pArr[i] = t;
		if (indexArr) {
			t = *pArr[index];
			*pArr[index] = *pArr[i];
			*pArr[i] = t;
		}
	}
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
