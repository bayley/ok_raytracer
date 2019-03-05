#include <math.h>
#include <stdio.h>
#include "geom.h"

//vec3f
vec3f vec3f::operator+(const vec3f& u) {
	return vec3f(x + u.x, y + u.y, z + u.z);
}

vec3f vec3f::operator-(const vec3f& u) {
	return vec3f(x - u.x, y - u.y, z - u.z);
}

vec3f vec3f::operator*(float c) {
	return vec3f(x * c, y * c, z * c);
}

vec3f vec3f::operator*(const vec3f& u) {
	return vec3f(x * u.x, y * u.y, z * u.z);
}

vec3f vec3f::operator/(float c) {
	return vec3f(x / c, y / c, z / c);
}

vec3f vec3f::operator+=(const vec3f& u) {
	x += u.x; y += u.y; z += u.z;
	return *this;
}

vec3f vec3f::operator*=(float c) {
	x *= c; y *= c; z *= c;
	return *this;
}

vec3f vec3f::operator/=(float c) {
	x /= c; y /= c; z /= c;
	return *this;
}

vec3f vec3f::cross(const vec3f& u) {
	return vec3f(y * u.z - z * u.y, z * u.x - x * u.z, x * u.y - y * u.x);
}

float vec3f::dot(const vec3f& u) {
	return x * u.x + y * u.y + z * u.z;
}

float vec3f::abs() {
	return sqrtf(x * x + y * y + z * z);
}

void vec3f::normalize() {
	float a = abs();
	if (a == 0.f) return;
	x /= a; y /= a; z /= a;
}

//Camera
Camera::Camera() {
	update(0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 1000, 1000);
}
Camera::Camera(float ex, float ey, float ez, float dx, float dy, float dz, float theta, int w, int h) {
	update(ex, ey, ez, dx, dy, dz, theta, w, h);
}

vec3f Camera::lookat(int x, int y) {
	x -= width / 2;
	y -= height / 2;
	return dir + u * x + v * y;
}

void Camera::move(float x, float y, float z) {
	update(x, y, z, dir.x, dir.y, dir.z, fov, width, height);
}

void Camera::point(float x, float y, float z) {
	update(eye.x, eye.y, eye.z, x, y, z, fov, width, height);
}

void Camera::zoom(float theta) {
	update(eye.x, eye.y, eye.z, dir.x, dir.y, dir.z, theta, width, height);
}

void Camera::resize(int w, int h) {
	update(eye.x, eye.y, eye.z, dir.x, dir.y, dir.z, fov, w, h);
}

void Camera::update(float ex, float ey, float ez, float dx, float dy, float dz, float theta, int w, int h) {
	eye.x = ex; eye.y = ey; eye.z = ez;
	dir.x = dx; dir.y = dy; dir.z = dz;
	fov = theta;

	dir.normalize();
	float dist2 = sqrtf(dir.x * dir.x + dir.y * dir.y);
	bool zdir = false;
	if (dist2 == 0.f) {dist2 = 1.f; zdir = true;}
	float beta = 2.f / (float)(w) / dist2 * tanf(theta / 2.f);

	if (zdir) {
		u.x = beta; u.y = 0.f; u.z = 0.f;
	} else {
		u.x = -beta * dir.y; u.y = beta * dir.x; u.z = 0.f;
	}
	
	v = dir.cross(u);
	v *= u.abs() / v.abs() * (float)h/(float)w;

	width = w;
	height = h;
}
