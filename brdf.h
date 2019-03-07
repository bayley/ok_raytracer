#ifndef __BRDF_H
#define __BRDF_H

#include "geom.h"

typedef vec3f (*brdf_t)(float, float, float, float);

//uniform BRDF
vec3f brdf_lambert(float th, float td, float ph, float pd);

//blinn-phong
vec3f brdf_blinn_phong(float th, float td, float ph, float pd);
#endif
