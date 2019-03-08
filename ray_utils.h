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

inline vec3f random_dir(vec3f n) {
  vec3f u = local_u(n);
  vec3f v = n.cross(u);

  float hit_theta = acosf(randf());
	float hit_phi = rangle() * 2.f;

  float c_u = sinf(hit_theta) * cosf(hit_phi);
  float c_v = sinf(hit_theta) * sinf(hit_phi);
  float c_n = cosf(hit_theta);

  vec3f out_dir = u * c_u + v * c_v + n * c_n;
  out_dir.normalize();

  return out_dir;
}

inline void get_angles(vec3f in, vec3f out, vec3f n, float * th, float * td, float * ph, float * pd) {
	vec3f h, u, v;
	in = in * -1.f;
	h = (in + out);
	h.normalize();
	u = local_u(n);
	v = n.cross(u);

	//h in n-u-v frame
	float h_u, h_v, h_n;
	h_u = h.dot(u);
	h_v = h.dot(v);
	h_n = h.dot(n);

	//in in n-u-v frame
	float i_u, i_v, i_n;
	i_u = in.dot(u);
	i_v = in.dot(v);
	i_n = in.dot(n);	

	vec3f h_nuv = {h_u, h_v, h_n}, i_nuv = {i_u, i_v, i_n};
	vec3f n_nuv = {0.f, 0.f, 1.f}, v_nuv = {0.f, 1.f, 0.f};

	vec3f tmp, diff;
	tmp = rotate(i_nuv, n_nuv, -*ph);
	diff = rotate(tmp, v_nuv, -*th);

	*th = acos(h_n);
	*ph = atan2(h_v, h_u);	

	*td = acos(diff.z);
	*pd = atan2(diff.y, diff.x);	
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
	theta = M_PI - acos(p.z / r);
	phi = atan2(p.x, p.y) + M_PI;

	if (theta < 0.f) theta += M_PI;
	if (theta >=  M_PI) theta -= M_PI;
	if (phi < 0.f) phi += 2.f * M_PI;
	if (phi >= 2.f * M_PI) phi -= 2.f * M_PI;

	result.x = phi / (2.f * M_PI);
	result.y = theta / M_PI;
	return result;
}

#endif
