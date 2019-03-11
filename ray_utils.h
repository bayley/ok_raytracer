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

inline float randf() {
	float u = (float)((double)(rand()) / (double)(RAND_MAX));
	return fminf(u, 1.f - FLT_EPSILON);
}

inline vec3f random_diffuse(float * pdf, vec3f n) {
  vec3f u = local_u(n);
  vec3f v = n.cross(u);

	float e = randf();

	float cos_theta = sqrtf(1.f - e);
	float sin_theta = sqrtf(e);

	float hit_phi = 2.f * M_PI * randf();

	float c_u = sin_theta * cosf(hit_phi);
	float c_v = sin_theta * sinf(hit_phi);

	vec3f out_dir = u * c_u + v * c_v + n * cos_theta;
	*pdf = cos_theta / M_PI;

  return out_dir;
}

inline vec3f random_specular(float * pdf, float roughness, vec3f view, vec3f n) {
  vec3f u = local_u(n);
  vec3f v = n.cross(u);

	float alpha = fmaxf(0.001f, roughness * roughness);
	float a2 = alpha * alpha;
	float e = randf();

	float cos_theta_h = sqrtf((1.f - e) / (1 + (a2 - 1.f) * e));
	float sin_theta_h = sqrtf(1.f - cos_theta_h * cos_theta_h);
	
	float hit_phi = 2.f * M_PI * randf();

  float c_u = sin_theta_h * cosf(hit_phi);
  float c_v = sin_theta_h * sinf(hit_phi);

	vec3f half = u * c_u + v * c_v + n * cos_theta_h;
	vec3f out_dir = half * 2.f * view.dot(half) - view;

	float t = 1.f + (a2 - 1.f) * cos_theta_h * cos_theta_h;
	*pdf = a2 / M_PI * 1.f / (t * t) * cos_theta_h / (4.f * out_dir.dot(half));

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
	
	float sdisc = sqrtf(disc);
	float t;
	t = (-b - sdisc) / (2.f * a);
	if (t < 0.f) t = (-b + sdisc) / (2.f * a);

	vec3f p = eval_ray(rh, t);
	float theta, phi;
	theta = M_PI - acos(p.z / r);
	phi = atan2(p.x, p.y) + M_PI;

	if (theta < 0.f) theta += M_PI;
	if (theta >= M_PI) theta -= M_PI;
	if (phi < 0.f) phi += 2 * M_PI;
	if (phi >= 2 * M_PI) phi -= 2 * M_PI;

	return {phi / (2.f * (float)M_PI), theta / (float)M_PI, 0.f};
}

#endif
