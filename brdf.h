#ifndef __BRDF_H
#define __BRDF_H

#include "geom.h"

//Principled BRDF object (with color)
class PrincipledBRDF {
public:
	PrincipledBRDF();
public:
	//vec3f sample(vec3f in, vec3f out, vec3f n);
	vec3f sample(float ci, float co, float c_th, float c_td);
public:
	float subsurface, metallic, specular, speculartint, roughness, anisotropic, sheen, sheentint, clearcoat, clearcoatgloss;
	vec3f base_color;
};

#endif
