#ifndef __BUFFERS_H
#define __BUFFERS_H

#include "geom.h"

class FBuffer {
public:
	FBuffer();
	FBuffer(int w, int h);
	~FBuffer() {free(data);}
public:
	void initialize(int w, int h);
public:
	float* operator[](int row);
public:
	void write(char * fname, float scale);
public:
	int width, height;
private:
	float * data;
};

class IBuffer {
public:
	IBuffer();
	IBuffer(int w, int h);
	~IBuffer() {free(data);}
public:
	void initialize(int w, int h);
public:
	int* operator[](int row);
public:
	void write(char * fname, float scale);
public:
	int width, height;
private:
	int * data;
};

class VBuffer {
public:
	VBuffer();
	VBuffer(int w, int h);
	~VBuffer() {free(data);}
public:
	void initialize(int w, int h);
public:
	vec3f* operator[](int row);
public:
	void write(char * fname, float scale, float gamma=1.f);
public:
	int width, height;
private:
	vec3f * data;
};
#endif
