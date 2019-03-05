#ifndef __BRDF_H
#define __BRDF_H

#include "geom.h"

typedef float (*brdf_t)(float, float, float, float);

//uniform BRDF
float brdf_lambert(float theta_i, float phi_i, float theta_o, float phi_o);

#endif
