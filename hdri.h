#ifndef __HRDI_H
#define __HDRI_H

#include "geom.h"

class HDRI {
public:
	HDRI(int w, int h, bool v=false);
public:
	int load(char * fname);
	int write(char * fname);
	int write_avg(char * fname);
	int write_var(char * fname, float scale=1.f);
public:
	void set(int row, int col, vec3f value);
	void add(int row, int col, vec3f value, int k=1);
public:
	vec3f at(int row, int col);	
	vec3f var(int row, int col);
	vec3f avg(int row, int col);
	int depth(int row, int col);
public:
	int width, height;
private:
	int write_buffer(vec3f * buf, char * fname, bool gamma, float scale=1.f);
private:
	vec3f *_buf, *_var, *_avg, *_ssq;
	int * _depth;
	bool track_variance;
};

#endif