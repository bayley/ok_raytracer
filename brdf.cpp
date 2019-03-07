#include <math.h>

#include "geom.h"
#include "brdf.h"

vec3f brdf_lambert(float th, float td, float ph, float pd) {
	return {1.f, 1.f, 1.f};
}

vec3f brdf_blinn_phong(float th, float td, float ph, float pd) {
	float c = cosf(th);
	c = 2.f * powf(c, 4.f);
	return {1.f + c, 1.f + c, 1.f + c};
}
