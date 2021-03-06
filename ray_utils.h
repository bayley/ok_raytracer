#ifndef __RAY_UTILS_H
#define __RAY_UTILS_H

#include <stdlib.h>
#include <random>
#include <math.h>
#include <thread>

#include <embree3/rtcore.h>

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

inline vec3f eval_ray(RTCRayHit * rh, float t) {
  vec3f result;
  result.x = rh->ray.org_x + t * rh->ray.dir_x;
  result.y = rh->ray.org_y + t * rh->ray.dir_y;
  result.z = rh->ray.org_z + t * rh->ray.dir_z;
  return result;
}

inline vec3f intersect_backdrop(RTCRayHit * rh, float r) {
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
