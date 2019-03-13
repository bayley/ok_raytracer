#ifndef __HDRI_H
#define __HDRI_H

#include "buffers.h"
#include "geom.h"

class HDRI {
public:
	HDRI();
	HDRI(int w, int h);
public:
	void initialize(int w, int h);
public:
	vec3f * operator[](int row);
public:
	int load(char * fname);
public:
	int width, height;
private:
	VBuffer data;
};

class RenderBuffer {
public:
	RenderBuffer(int w, int h);
public:
	void add(int row, int col, vec3f value, int count = 1);
public:
	int width, height;
	VBuffer average, variance;
	IBuffer depth;
private:
	VBuffer total, ssq;
};

#endif
