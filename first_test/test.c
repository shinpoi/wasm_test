#include<stdio.h>
#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE
void testWASM(unsigned char* buffer, unsigned int len){
	for (int i=0; i<len; i++){
		buffer[i] = 255 - buffer[i];
	}
}

/*
void testWASM2(unsigned char* buffer_in,
			  unsigned char* buffer_out,
			  unsigned int width,
			  unsigned int height) {
	;
}*/
