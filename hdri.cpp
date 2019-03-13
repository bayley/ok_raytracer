#include <stdlib.h>
#include <stdio.h>

#include "buffers.h"
#include "geom.h"
#include "hdri.h"
#include "bmp.h"

typedef unsigned char u8;

HDRI::HDRI() {
	width = 0; height = 0;
}

HDRI::HDRI(int w, int h) {
	initialize(w, h);
}

void HDRI::initialize(int w, int h) {
	width = w; height = h;
	data.initialize(width, height);
}

vec3f* HDRI::operator[](int row) {
	return data[row];
}

int HDRI::load(char * fname) {
	u8 *red, *green, *blue;
	red = (u8*)malloc(width * height * sizeof(u8));
	green = (u8*)malloc(width * height * sizeof(u8));
	blue = (u8*)malloc(width * height * sizeof(u8));

	if(read_bmp(red, green, blue, width, height, fname) < 0) return -1;

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			data[row][col] = {(float)red[row * width + col], (float)green[row * width + col], (float)blue[row * width + col]};
			data[row][col] /= 255.f;
			data[row][col] = data[row][col].pow(2.2f);
		}
	}

	return 1;
}

RenderBuffer::RenderBuffer(int w, int h) {
	width = w; height = h;
	average.initialize(width, height);
	variance.initialize(width, height);
	depth.initialize(width, height);
	total.initialize(width, height);
	ssq.initialize(width, height);
}

void RenderBuffer::add(int row, int col, vec3f value, int count) {
	total[row][col] += value * count;
	ssq[row][col] += value * value * (float)count;
	depth[row][col] += count;
	average[row][col] = total[row][col] / (float)depth[row][col];
	variance[row][col] = (ssq[row][col] - total[row][col] * total[row][col] / (float)depth[row][col]) / (float)depth[row][col];
}
