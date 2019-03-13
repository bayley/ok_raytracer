#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "hdri.h"
#include "bmp.h"
#include "geom.h"

HDRI::HDRI(int w, int h, bool v) {
	width = w;
	height = h;
	track_variance = v;
	_buf = (vec3f *) malloc(width * height * sizeof(vec3f));

	for (int i = 0; i < width * height; i++) _buf[i] = {0.f, 0.f, 0.f};
	if (track_variance) {
		_var = (vec3f *) malloc(width * height * sizeof(vec3f));
		_avg = (vec3f *) malloc(width * height * sizeof(vec3f));
		_ssq = (vec3f *) malloc(width * height * sizeof(vec3f));
		_depth = (int*)malloc(width * height * sizeof(int));

		for (int i = 0; i < width * height; i++) {
			_var[i] = {0.f, 0.f, 0.f};
			_avg[i] = {0.f, 0.f, 0.f};
			_ssq[i] = {0.f, 0.f, 0.f};
			_depth[i] = 0;
		}
	}
}

int HDRI::load(char * fname) {
	unsigned char *red, *green, *blue;

  red = (unsigned char *)malloc(width * height);
  green = (unsigned char *)malloc(width * height);
  blue = (unsigned char *)malloc(width * height);

  if (read_bmp(red, green, blue, width, height, fname) < 0) return -1;

  for (int i = 0; i < width * height; i++) {
    _buf[i] = {(float)red[i], (float)green[i], (float)blue[i]};
    _buf[i] /= 255.f;
    _buf[i] = _buf[i].pow(2.2f);
  }

  free(red); free(green); free(blue);
  return 1;
}

int HDRI::write_buffer(vec3f * buf, char * fname, bool gamma, float scale) {
	unsigned char *red, *green, *blue;

  red = (unsigned char *)malloc(width * height);
  green = (unsigned char *)malloc(width * height);
  blue = (unsigned char *)malloc(width * height);

	for (int i = 0; i < width * height; i++) {
		vec3f px = buf[i] * scale;
		if (gamma) px = px.pow(1.f / 2.2f);
		px *= 256.f;

		if (px.x > 255.f) px.x = 255.f;
		if (px.y > 255.f) px.y = 255.f;
		if (px.z > 255.f) px.z = 255.f;

		red[i] = (unsigned char) px.x;
		green[i] = (unsigned char) px.y;
		blue[i] = (unsigned char) px.z;
	}	

	write_bmp(red, green, blue, width, height, fname);
	free(red); free(green); free(blue);

	return 1;
}

int HDRI::write(char * fname) {
	return write_buffer(_buf, fname, true);
}

int HDRI::write_avg(char * fname) {
	if (!track_variance) return -1;
	return write_buffer(_avg, fname, true);
}

int HDRI::write_var(char * fname, float scale) {
	if (!track_variance) return -1;
	return write_buffer(_var, fname, false, scale);
}

void HDRI::set(int row, int col, vec3f value) {
	_buf[row * width + col] = value;
}

void HDRI::add(int row, int col, vec3f value, int k) {
	int i = row * width + col;

	_buf[i] += value * (float)k;
	
	if (track_variance) {
		_ssq[i] += value * value * (float)k;
		_depth[i] += k;

		_avg[i] = _buf[i] / (float)_depth[i];
		_var[i] = (_ssq[i] - _buf[i] * _buf[i] / (float)_depth[i]) / (float)_depth[i];	
	}
}

vec3f HDRI::at(int row, int col) {
	return _buf[row * width + col];
}

vec3f HDRI::var(int row, int col) {
	return _var[row * width + col];
}

vec3f HDRI::avg(int row, int col) {
	return _avg[row * width + col];
}

int HDRI::depth(int row, int col) {
	return _depth[row * width + col];	
}
