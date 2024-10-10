#include "simd.h"

void memcpy(uint8_t *dest, uint8_t* src, int size) {
	const int stride = 32;

	if (size < stride) {
		for(int i = 0; i < size; ++i) {
			dest[i] = src[i];
		}
	} else {
		uint32_t offset = 0;
		while (offset + 32 < size) {
		}
	}
}
