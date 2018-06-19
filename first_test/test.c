#include<stdio.h>
#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE
void testWASM(unsigned char* buffer, unsigned int len){
	for (int i=0; i<len; i=i+4){
		buffer[i] = 255 - buffer[i];
		buffer[i+1] = 255 - buffer[i+1];
		buffer[i+2] = 255 - buffer[i+2];
	}
}

/*
void testWASM2(unsigned char* buffer_in,
			  unsigned char* buffer_out,
			  unsigned int width,
			  unsigned int height) {
	;
}*/
