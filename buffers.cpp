#include <stdio.h>
#include <stdlib.h>

#include "buffers.h"
#include "bmp.h"

typedef unsigned char u8;

u8 clip(float f) {
	if (f < 256.f) return (u8)f;
	return 255;
}

u8 clip(int n) {
	if (n < 256) return (u8)n;
	return 255;
}

FBuffer::FBuffer() {
	width = 0; height = 0;
	data = NULL;
}

FBuffer::FBuffer(int w, int h) {
	initialize(w, h);
}

void FBuffer::initialize(int w, int h) {
	width = w; height = h;
	data = (float*)malloc(width * height * sizeof(float));
	for (int i = 0; i < width * height; i++) data[i] = 0.f;
}

float* FBuffer::operator[](int row) {
	return data + row * width;
}

void FBuffer::write(char * fname, float scale) {
	u8 *red, *green, *blue;
	red = (u8*) malloc(width * height * sizeof(u8));
	green = (u8*) malloc(width * height * sizeof(u8));
	blue = (u8*) malloc(width * height * sizeof(u8));

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			red[row * width + col] = clip(scale * data[row * width + col]);
			green[row * width + col] = clip(scale * data[row * width + col]);
			blue[row * width + col] = clip(scale * data[row * width + col]);
		}
	}

	write_bmp(red, green, blue, width, height, fname);
	free(red); free(green); free(blue);
}

IBuffer::IBuffer() {
	width = 0; height = 0;
	data = NULL;
}

IBuffer::IBuffer(int w, int h) {
	initialize(w, h);
}

void IBuffer::initialize(int w, int h) {
	width = w; height = h;
	data = (int*)malloc(width * height * sizeof(int));
	for (int i = 0; i < width * height; i++) data[i] = 0;
}

int* IBuffer::operator[](int row) {
	return data + row * width;
}

void IBuffer::write(char * fname, float scale) {
	u8 *red, *green, *blue;
	red = (u8*) malloc(width * height * sizeof(u8));
	green = (u8*) malloc(width * height * sizeof(u8));
	blue = (u8*) malloc(width * height * sizeof(u8));

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			red[row * width + col] = clip(scale * (float)data[row * width + col]);
			green[row * width + col] = clip(scale * (float)data[row * width + col]);
			blue[row * width + col] = clip(scale * (float)data[row * width + col]);
		}
	}

	write_bmp(red, green, blue, width, height, fname);
	free(red); free(green); free(blue);
}

VBuffer::VBuffer() {
	width = 0; height = 0;
	data = NULL;
}

VBuffer::VBuffer(int w, int h) {
	initialize(w, h);
}

void VBuffer::initialize(int w, int h) {
	width = w; height = h;
	data = (vec3f*)malloc(width * height * sizeof(vec3f));
	for (int i = 0; i < width * height; i++) data[i] = {0.f, 0.f, 0.f};
}

vec3f* VBuffer::operator[](int row) {
	return data + row * width;
}

void VBuffer::write(char * fname, float scale, float gamma) {
	u8 *red, *green, *blue;
	red = (u8*) malloc(width * height * sizeof(u8));
	green = (u8*) malloc(width * height * sizeof(u8));
	blue = (u8*) malloc(width * height * sizeof(u8));

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			red[row * width + col] = clip(scale * powf(data[row * width + col].x, 1.f / gamma));
			green[row * width + col] = clip(scale * powf(data[row * width + col].y, 1.f / gamma));
			blue[row * width + col] = clip(scale * powf(data[row * width + col].z, 1.f / gamma));
		}
	}

	write_bmp(red, green, blue, width, height, fname);
	free(red); free(green); free(blue);
}
