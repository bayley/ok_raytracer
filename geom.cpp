#include <math.h>
#include <stdio.h>
#include "geom.h"

//matrix3f
matrix3f::matrix3f(float a11, float a12, float a13,
									 float a21, float a22, float a23,
									 float a31, float a32, float a33) {
	data[1][1] = a11; data[1][2] = a12; data[1][3] = a13;
	data[2][1] = a21; data[2][2] = a22; data[2][3] = a23;
	data[3][1] = a31; data[3][2] = a32; data[3][3] = a33;
}

vec3f * matrix3f::mul(vec3f * x) {
	vec3f * result = new vec3f;
	result->x = data[1][1] * x->x + data[1][2] * x->y + data[1][3] * x->z;
	result->y = data[2][1] * x->x + data[2][2] * x->y + data[2][3] * x->z;
	result->z = data[3][1] * x->x + data[3][2] * x->y + data[3][3] * x->z;
	return result;
}

matrix3f * matrix3f::mul(float c) {
	matrix3f * result = new matrix3f;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result->set(i, j, c * data[i][j]);
		}
	}
	return result;
}

float matrix3f::at(int i, int j) {return data[i][j];}
void matrix3f::set(int i, int j, float x) {data[i][j] = x;}

//vec3f
void vec3f::normalize() {
	float a = abs();
	if (a == 0.f) return;
	x /= a; y /= a; z /= a;
}

vec3f * vec3f::cross(vec3f *b) {
	return new vec3f(y * b->z - z * b->y, z * b->x - x * b->z, x * b->y - y * b->x);
}

//Camera
Camera::Camera(float ex, float ey, float ez, float dx, float dy, float dz, float theta, int w, int h) {
	update(ex, ey, ez, dx, dy, dz, theta, w, h);
}

vec3f * Camera::lookat(int x, int y) {
	x -= width / 2;
	y -= height / 2;
	vec3f * result =  add(dir, add(mul(u, x), mul(v, y)));
	return result;
}

void Camera::move(vec3f * e) {
	update(e->x, e->y, e->z, dir->x, dir->y, dir->z, fov, width, height);
}

void Camera::point(vec3f * d) {
	update(eye->x, eye->y, eye->z, d->x, d->y, d->z, fov, width, height);
}

void Camera::zoom(float theta) {
	update(eye->x, eye->y, eye->z, dir->x, dir->y, dir->z, theta, width, height);
}

void Camera::resize(int w, int h) {
	update(eye->x, eye->y, eye->z, dir->x, dir->y, dir->z, fov, w, h);
}

void Camera::update(float ex, float ey, float ez, float dx, float dy, float dz, float theta, int w, int h) {
	eye = new vec3f(ex, ey, ez);
	dir = new vec3f(dx, dy, dz);
	fov = theta;

	dir->normalize();
	float dist2 = sqrtf(dir->x * dir->x + dir->y * dir->y);
	bool zdir = false;
	if (dist2 == 0.f) {dist2 = 1.f; zdir = true;}
	float beta = 2.f / (float)(w) / dist2 * tanf(theta / 2.f);

	if (zdir) {
		u = new vec3f(beta, 0.f, 0.f);
	} else {
		u = new vec3f(-beta * dir->y, beta * dir->x, 0.f);
	}
	
	v = dir->cross(u);
	v = mul(v, u->abs() / v->abs() * (float)h/(float)w);

	width = w;
	height = h;
}

//utility
vec3f * mul(vec3f * v, float c) {
	return new vec3f(c * v->x, c * v->y, c * v->z);
}

vec3f * mul(vec3f * u, vec3f * v) {
	return new vec3f(u->x * v->x, u->y * v->y, u->z * v->z);
}

vec3f * add(vec3f * u, vec3f * v) {
	return new vec3f(u->x + v->x, u->y + v->y, u->z + v->z);
}

vec3f * sub(vec3f * u, vec3f * v) {
	return add(u, mul(v, -1));
}

matrix3f * rotation(float theta, int axis) {
	switch (axis) {
	case AXIS_X:
		return new matrix3f(1.f, 0.f, 0.f,
												0.f, cosf(theta), -sinf(theta),
												0.f, sinf(theta), cosf(theta));
	case AXIS_Y:
		return new matrix3f(cosf(theta), 0.f, sinf(theta),
											 	0.f, 1.f, 0.f,
												-sinf(theta), 0.f, cosf(theta));
	case AXIS_Z:
		return new matrix3f(cosf(theta), -sinf(theta), 0.f,
												sinf(theta), cosf(theta), 0.f,
												0.f, 0.f, 1.f);
	default:
		return new matrix3f(1.f, 0.f, 0.f,
												0.f, 1.f, 0.f,
												0.f, 0.f, 1.f);
	}
}
