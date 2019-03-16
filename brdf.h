#ifndef __BRDF_H
#define __BRDF_H

#include "geom.h"

//utility functions
vec3f lerp(vec3f a, vec3f b, float t);
float lerp(float a, float b, float t);

//Principled BRDF object (with color)
class PrincipledBRDF {
public:
	PrincipledBRDF();
public:
	vec3f sample_diffuse(float ci, float co, float c_th, float c_td);
	vec3f sample_specular(float ci, float co, float c_th, float c_td);
	vec3f sample(float ci, float co, float c_th, float c_td);
public:
	float subsurface, metallic, specular, speculartint, roughness, anisotropic, sheen, sheentint, clearcoat, clearcoatgloss;
	vec3f base_color;
};

#endif
