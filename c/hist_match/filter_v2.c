#include<stdint.h>
#include<stdlib.h>

void hist_match(uint8_t* srcArr, uint8_t* refArr, uint64_t srcLen, uint64_t refLen);
void hist(uint8_t* arr, uint64_t len, double* histArr, uint64_t histLen);
double sum(double* arr, uint64_t len);
void hist2map(double* histArr, uint64_t histLen);
void calc_map(double* srcMap, double* refMap, uint64_t* map, uint64_t mapLen);

void colorExchange(uint8_t* buffer_src, uint8_t* buffer_ref, uint64_t len_src, uint64_t len_ref);
void sort(uint8_t* arr_src, uint8_t* arr_with, int offset, int step);



unsigned char* cr_buffer(uint64_t size) {
	return malloc(size);
}

void fr_buffer(unsigned char* p) {
	free(p);
}

void render_as_RGB(uint8_t* srcBuffer, uint8_t* refBuffer, uint64_t srcLen, uint64_t refLen) {
	uint8_t srcArrR[srcLen/4], srcArrG[srcLen/4], srcArrB[srcLen/4];
	uint8_t refArrR[refLen/4], refArrG[refLen/4], refArrB[refLen/4];

	// init srcArrRGB
	for (int i=0; i<srcLen; i+=4) {
		srcArrR[i/4] = srcBuffer[i];
		srcArrG[i/4] = srcBuffer[i+1];
		srcArrB[i/4] = srcBuffer[i+2];
	}
	
	// init refArrRGB
	for (int i=0; i<refLen; i+=4) {
		refArrR[i/4] = refBuffer[i];
		refArrG[i/4] = refBuffer[i+1];
		refArrB[i/4] = refBuffer[i+2];
	}
	
	hist_match(srcArrR, refArrR, srcLen/4, refLen/4);
	hist_match(srcArrG, refArrG, srcLen/4, refLen/4);
	hist_match(srcArrB, refArrB, srcLen/4, refLen/4);

	// return data
	for (int i=0; i<srcLen; i+=4) {
		srcBuffer[i] = srcArrR[i/4];
		srcBuffer[i+1] = srcArrG[i/4];
		srcBuffer[i+2] = srcArrB[i/4];
	}
}

void hist_match(uint8_t* srcArr, uint8_t* refArr, uint64_t srcLen, uint64_t refLen) {
	double srcHist[256] = {0};
	double refHist[256] = {0};
	uint64_t map[256] = {0};

	hist(srcArr, srcLen, srcHist, 256);
	hist2map(srcHist, 256);
	hist(refArr, refLen, refHist, 256);
	hist2map(refHist, 256);

	calc_map(srcHist, refHist, map, 256);
}


// double hist_R/G/B[256] = {0}
void hist2map(double* histArr, uint64_t histLen) {
	double summ = sum(histArr, histLen);

	for (int i=0; i<histLen; i++) {
		histArr[i] = histArr[i]/summ;
	}

	double sum = 0;
	for (int i=0; i<histLen; i++) {
		sum += histArr[i];
		histArr[i] = sum;
	}
}


void calc_map(double* srcMap, double* refMap, uint64_t* map, uint64_t mapLen) {
	for (int i=0; i<mapLen; i++){
		double minDiff = 2;
		double diff = 0;
		for (int j=map[i-1]; j<mapLen; j++) {
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
	}
}

double sum(double* arr, uint64_t len) {
	double summ = 0;
	for (int i=0; i<len; i++){
		summ += arr[i];
	}
	return summ;
}
/*
void colorExchange(uint8_t* buffer_src, uint8_t* buffer_ref, uint64_t len_src, uint64_t len_ref) {
	uint8_t src_enmu[len_src];
	
	for (int i=0; i<len_src; i++) {
		src_enmu[i] = i;
		src_enmu[i+1] = i;
		src_enmu[i+2] = i;
	}

	sort(buffer_src, src_enmu, 0, 4);
	sort(buffer_src, src_enmu, 1, 4);
	sort(buffer_src, src_enmu, 2, 4);

	// hist ref
	int hist_R[256] = {0};
	int hist_G[256] = {0};
	int hist_B[256] = {0};
	for (int i=0; i<len_ref; i++) {
		hist_R[buffer_ref[i]]++;
		hist_R[buffer_ref[i+1]]++;
		hist_R[buffer_ref[i+2]]++;
	}

	// expand/reduce ref size as same as src size;
	int max_R, max_G, max_B;
	max_R = max_G = max_B = 0;
	double scale = len_src/len_ref;
	for (int i=0; i<256; i++) {
		hist_R[i] = (int)(hist_R[i]*scale + 0.5);
		hist_G[i] = (int)(hist_G[i]*scale + 0.5);
		hist_B[i] = (int)(hist_B[i]*scale + 0.5);
		if (hist_R[i] > hist_R[max_R]) {max_R = i;}
		if (hist_G[i] > hist_G[max_R]) {max_G = i;}
		if (hist_B[i] > hist_B[max_R]) {max_B = i;}
	}

	double sum_hist_R = sum(hist_R, 256);
	double sum_hist_G = sum(hist_G, 256);
	double sum_hist_B = sum(hist_B, 256);
	hist_R[max_R] += len_src/4 - sum_hist_R;
	hist_R[max_G] += len_src/4 - sum_hist_G;
	hist_R[max_B] += len_src/4 - sum_hist_B;
	
	//buffer_ref --> buffer_src
	int r=0, g=0, b=0;
	for (int i=0; i<len_src; i+=4) {
		if (hist_R[r] < 0) {r++;}
		if (hist_G[g] < 0) {g++;}
		if (hist_B[b] < 0) {b++;}
		buffer_src[i] = r;
		buffer_src[i+1] = g;
		buffer_src[i+2] = b;
		hist_R[r]--;
		hist_G[g]--;
		hist_B[b]--;
	}
	
	//sort(enmu + buffer_src)
	sort(src_enmu, buffer_src, 0, 4);
	sort(src_enmu, buffer_src, 1, 4);
	sort(src_enmu, buffer_src, 2, 4);
}

void sort(uint8_t* arr_src, uint8_t* arr_with, int offset, int step) {
	;
}
*/
// RGBtoHSV()
// exchangeHSV
// matchHSV
