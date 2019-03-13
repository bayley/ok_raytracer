#ifndef __BUFFERS_H
#define __BUFFERS_H

#include "geom.h"

class FBuffer {
public:
	FBuffer();
	FBuffer(int w, int h);
public:
	void initialize(int w, int h);
public:
	float* operator[](int row);
public:
	void write(char * fname, float scale);
private:
	int width, height;
	float * data;
};

class IBuffer {
public:
	IBuffer();
	IBuffer(int w, int h);
public:
	void initialize(int w, int h);
public:
	int* operator[](int row);
public:
	void write(char * fname, float scale);
private:
	int width, height;
	int * data;
};

class VBuffer {
public:
	VBuffer();
	VBuffer(int w, int h);
public:
	void initialize(int w, int h);
public:
	vec3f* operator[](int row);
public:
	void write(char * fname, float scale, float gamma=1.f);
private:
	int width, height;
	vec3f * data;
};
#endif
