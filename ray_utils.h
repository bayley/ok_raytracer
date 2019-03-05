#ifndef __RAY_UTILS_H
#define __RAY_UTILS_H

#include <stdlib.h>
#include <embree3/rtcore.h>
#include <math.h>

#include "geom.h"

inline void set_dir(RTCRayHit * rh, vec3f u) {
  rh->ray.dir_x = u.x;
  rh->ray.dir_y = u.y;
  rh->ray.dir_z = u.z;
}

inline void set_org(RTCRayHit * rh, vec3f u) {
  rh->ray.org_x = u.x;
  rh->ray.org_y = u.y;
  rh->ray.org_z = u.z;
}

inline vec3f local_u(vec3f hit_n) {
  vec3f hit_u = {-hit_n.y, hit_n.x, 0.f};
  if (hit_u.abs() == 0.f) {
		hit_u = {0.f, -hit_n.z, hit_n.y};
  }
  hit_u.normalize();
  return hit_u;
}

inline float rangle() { 
  return (float)(M_PI * (double)(rand())/(double)(RAND_MAX));
}

inline float randf() {
	return (float)((double)(rand()) / (double)(RAND_MAX));
}

inline vec3f random_dir(vec3f n, float backside) {
  vec3f u = local_u(n);
  vec3f v = n.cross(u);

  float hit_theta = rangle() - M_PI / 2.f, hit_phi = rangle() * 2.f;

  float c_u = sinf(hit_theta) * cosf(hit_phi);
  float c_v = sinf(hit_theta) * sinf(hit_phi);
  float c_n = cosf(hit_theta) * backside;

  vec3f out_dir = u * c_u + v * c_v + n * c_n;
  out_dir.normalize();

  return out_dir;
}

inline vec3f eval_ray(RTCRayHit * rh, float t) {
  vec3f result;
  result.x = rh->ray.org_x + t * rh->ray.dir_x;
  result.y = rh->ray.org_y + t * rh->ray.dir_y;
  result.z = rh->ray.org_z + t * rh->ray.dir_z;
  return result;
}

inline vec3f intersect_hdri(RTCRayHit * rh, float r) {
	float ox = rh->ray.org_x; float oy = rh->ray.org_y; float oz = rh->ray.org_z;
	float dx = rh->ray.dir_x; float dy = rh->ray.dir_y; float dz = rh->ray.dir_z;

	float a = dx * dx + dy * dy + dz * dz;
	float b = 2.f * ox * dx + 2.f * oy * dy + 2.f * oz * dz;
	float c = ox * ox + oy * oy + oz * oz - r * r;
	float disc = b * b - 4 * a * c;
	
	vec3f result = {0.f, 0.f, 0.f};
	if (disc < 0) return result; //should not happen!

	float sdisc = sqrtf(disc);
	float t;
	t = (-b - sdisc) / (2.f * a);
	if (t < 0.f) t = (-b + sdisc) / (2.f * a);
	if (t < 0.f) return result; //should not happen!

	vec3f p = eval_ray(rh, t);
	float theta, phi;
	theta = acos(p.z / r);
	phi = atan2(p.x, p.y) + M_PI;

	if (theta >=  M_PI) theta -= M_PI;
	if (phi >= 2.f * M_PI) phi -= 2.f * M_PI;

	result.x = phi / (2.f * M_PI);
	result.y = theta / M_PI;
	return result;
}

#endif
