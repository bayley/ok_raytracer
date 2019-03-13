#ifndef __GEOM_H
#define __GEOM_H

#include <embree3/rtcore.h>
#include <math.h>

class vec3f;
class Camera;

class vec3f {
public:
	vec3f() {}
	vec3f(float u, float v, float w) {x = u; y = v; z = w;}
public:
	vec3f operator+(const vec3f& u);
	vec3f operator-(const vec3f& u);
	vec3f operator*(const vec3f& u);
	vec3f operator*(float c);
	vec3f operator/(float c);
	vec3f operator/(const vec3f& u);
public:
	vec3f operator+=(const vec3f& u);
	vec3f operator*=(float c);
	vec3f operator/=(float c);
public:
	vec3f pow(float c);
public:
	vec3f cross(const vec3f& u);
	float dot(const vec3f& u);
public:
	float abs();
	void normalize(); //modifies this vector!
public:
	float x; float y; float z;
};

class Camera {
public:
	Camera();
	Camera(float ex, float ey, float ez, float dx, float dy, float dz, float theta, int w, int h);
public:
	vec3f lookat(float x, float y);
public:
	void move(float x, float y, float z);
	void point(float x, float y, float z);
	void zoom(float theta);
	void resize(int w, int h);
private:
	void update(float ex, float ey, float ez, float dx, float dy, float dz, float theta, int w, int h);
public:
	vec3f eye, dir;
	float fov;
	int width, height;
private:
	vec3f u, v;
};

//these don't do much, so they are just structs
typedef struct {
	int v0; int v1; int v2;
} Triangle;

typedef struct {
	float x; float y; float z;
} Vertex;

enum {
	AXIS_X,
	AXIS_Y,
	AXIS_Z,
};

//utility functions
vec3f rotate(vec3f v, vec3f a, float theta);

#endif
