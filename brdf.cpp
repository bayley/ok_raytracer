#include <math.h>
#include <stdio.h>

#include "geom.h"
#include "brdf.h"

PrincipledBRDF::PrincipledBRDF() {
	subsurface = 0.f;
	metallic = 0.f;
	specular = 0.f;
	speculartint = 0.f;
	roughness = 1.f;
	anisotropic = 0.f;
	sheen = 0.f;
	sheentint = 0.f;
	clearcoat = 0.f;
	clearcoatgloss = 0.f;
	base_color = {1.f, 1.f, 1.f};
}

float GTR1(float alpha, float cos) {
	float norm = (alpha * alpha - 1.f) / (M_PI * log(alpha * alpha));
	float tmp = (alpha * alpha - 1.f) * cos * cos + 1.f;
	return norm / tmp;
}

float GTR2(float alpha, float cos) {
	float norm = alpha * alpha / M_PI;
	float tmp = (alpha * alpha - 1.f) * cos * cos + 1.f;
	return norm / (tmp * tmp);
}

float GGX(float alpha, float cos) {
	float a = alpha * alpha;
	float b = cos * cos;
	return 1.f / (cos + sqrt(a + b - a * b));
}

float schlick_F(float cos) {
	float u = 1.f - cos;
	float u2 = u * u;
	return u2 * u2 * u;
}

vec3f lerp(vec3f a, vec3f b, float t) {
	return a * (1.f - t) + b * t;
}

float lerp(float a, float b, float t) {
	return a * (1.f - t) + b * t;
}

vec3f PrincipledBRDF::sample(float ci, float co, float c_th, float c_td) {
	vec3f ones = {1.f, 1.f, 1.f};
	vec3f r_diffuse, r_sheen, r_specular, r_clearcoat;

	float F_co, F_ci, F_c_td;
	F_co = schlick_F(co);
	F_ci = schlick_F(ci);
	F_c_td = schlick_F(c_td);

	//diffuse
	float fd90 = .5f + 2.f * c_td * c_td * roughness;
	float fd = 1.f / M_PI * lerp(1.f, fd90, F_co) * lerp(1.f, fd90, F_ci);

	float fss90 = c_td * c_td * roughness;
	float fss = lerp(1.f, fss90, F_co) * lerp(1.f, fss90, F_ci);
	float ss = 1.25f / M_PI * (fss * (1.f / (co + ci) - .5f) + .5f);

	r_diffuse = base_color * lerp(fd, ss, subsurface) * (1.f - metallic);

	//sheen
	r_sheen = lerp(ones, base_color, sheentint) * sheen * F_c_td * (1.f - metallic);

	//specular F
	vec3f Fi_spec = lerp(ones, base_color, speculartint) * specular * 0.08f; //incident
	vec3f Fg_spec = ones * (1 - specular * 0.08f) * F_c_td; //grazing
	vec3f F_m = base_color * specular; //TODO: fresnel term for metals

	//specular D, G
	float alpha = fmaxf(roughness * roughness, 0.001f);
	float D_spec = GTR2(alpha, c_th);
	float G_spec = GGX(alpha, ci) * GGX(alpha, co);

	//clearcoat
	vec3f F_cc = ones * lerp(.04f, 1.f, F_c_td);
	float D_cc = GTR1(lerp(.1f, 0.001f, clearcoatgloss), c_th);
	float G_cc = GGX(.25f, ci) * GGX(.25f, co);

	//add them together, interpolating between dielectric and metallic
	r_specular = lerp(Fi_spec + Fg_spec, F_m, metallic) * D_spec * G_spec;
	r_clearcoat = F_cc * D_cc * G_cc * clearcoat * 0.25f;

	return r_diffuse + r_sheen + r_specular + r_clearcoat;
}
