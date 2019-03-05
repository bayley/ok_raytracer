#include <math.h>

#include "brdf.h"

float brdf_lambert(float theta_i, float phi_i, float theta_o, float phi_o) {
	return 1.f;
}

vec3f * emit_black(int id, float u, float v) {
	return new vec3f(0.f, 0.f, 0.f);
}
