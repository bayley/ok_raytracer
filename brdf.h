#ifndef __BRDF_H
#define __BRDF_H

#include "geom.h"

typedef vec3f (*brdf_t)(float, float, float, float, float, float);

//uniform BRDF
vec3f brdf_lambert(float ci, float co, float th, float td, float ph, float pd);

//WDAS principled BRDF
vec3f brdf_principled(float ci, float co, float th, float td, float ph, float pd);

//Principled BRDF object (with color)
class PrincipledBRDF {
public:
	PrincipledBRDF();
public:
	vec3f sample(float ci, float co, float th, float td, float ph, float pd);
public:
	float subsurface, metallic, specular, speculartint, roughness, anisotropic, sheen, sheentint, clearcoat, clearcoatgloss;
	vec3f base_color;
};

#endif
