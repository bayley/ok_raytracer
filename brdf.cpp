#include <math.h>

#include "geom.h"
#include "brdf.h"

vec3f brdf_lambert(float ci, float co, float th, float td, float ph, float pd) {
	return {1.f / M_PI, 1.f / M_PI, 1.f / M_PI};
}

float subsurface = 0.f;
float metallic = 0.f;
float specular = 1.f;
float speculartint = 0.f;
float roughness = 0.7f;
float anisotropic = 0.f;
float sheen = 0.f;
float sheentint = 0.f;
float clearcoat = 1.f;
float clearcoatgloss = 0.5f;

vec3f brdf_principled(float ci, float co, float th, float td, float ph, float pd) {
	float fd90 = .5f + 2.f * cosf(td) * cosf(td) * roughness;
	float fd = 1.f / M_PI * (1.f + (fd90 - 1.f) * powf(1 - co, 5.f)) * (1.f + (fd90 - 1.f) * powf(1 - ci, 5.f)); //diffuse	

	float c_th, s_th;
	c_th = cosf(th); s_th = sinf(th);
	
	float alpha_spec = roughness * roughness;	
	float alpha_cc = (1.f - clearcoatgloss) * (1.f - clearcoatgloss);

	float tmp_spec = alpha_spec * alpha_spec * c_th * c_th + s_th * s_th;
	float tmp_cc = alpha_cc * alpha_cc * c_th * c_th + s_th * s_th;

	float spec_gtr = 1.f / tmp_spec * tmp_spec;
	float cc_gtr = 0.25f * clearcoat / tmp_cc;

	float c_td = cosf(td);
	float cc_f = .04f + 0.96f * powf(1.f - c_td, 5.f);
	float spec_f = (specular * 0.08f) + (1 - specular * 0.08f) * powf(1.f - c_td, 5.f);
	float met_f = specular;

	float alpha_g = (0.5f * roughness / 2.f) * (0.5f + roughness / 2.f);
	float ch_g = c_td > 0.f ? 1.f : 0.f;
	float ti = sqrtf(1.f - ci * ci) / ci;

	float cc_g = ch_g * 2.f / (1.f + sqrtf(1.f + .15259f * ti * ti));
	float spec_g = ch_g * 2.f / (1.f + sqrtf(1.f + alpha_g * alpha_g * ti * ti));

	vec3f rd = {fd, fd, fd};
	vec3f rs = {spec_gtr * spec_f * spec_g, spec_gtr * spec_f * spec_g, spec_gtr * spec_f * spec_g};
	vec3f rm = {spec_gtr * met_f * spec_g, spec_gtr * met_f * spec_g, spec_gtr * met_f * spec_g};
	vec3f rcc = {cc_gtr * cc_f * cc_g, cc_gtr * cc_f * cc_g, cc_gtr * cc_f * cc_g};

	return rd * (1.f - metallic) + (rs * (1.f - metallic) + rm * metallic + rcc) / (4.f * ci * co);
}

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

vec3f PrincipledBRDF::sample(float ci, float co, float th, float td, float ph, float pd) {
	vec3f ones = {1.f, 1.f, 1.f};

	float fd90 = .5f + 2.f * cosf(td) * cosf(td) * roughness;
	float fd = 1.f / M_PI * (1.f + (fd90 - 1.f) * powf(1 - co, 5.f)) * (1.f + (fd90 - 1.f) * powf(1 - ci, 5.f)); //diffuse	

	float c_th, s_th;
	c_th = cosf(th); s_th = sinf(th);
	
	float alpha_spec = roughness * roughness;	
	float alpha_cc = (1.f - clearcoatgloss) * (1.f - clearcoatgloss);

	float tmp_spec = alpha_spec * alpha_spec * c_th * c_th + s_th * s_th;
	float tmp_cc = alpha_cc * alpha_cc * c_th * c_th + s_th * s_th;

	float spec_gtr = 1.f / tmp_spec * tmp_spec;
	float cc_gtr = 0.25f * clearcoat / tmp_cc;

	float c_td = cosf(td);
	float cc_f = .04f + 0.96f * powf(1.f - c_td, 5.f);
	vec3f spec_f_incident = (ones * (1.f - speculartint) + base_color * speculartint) * specular * 0.08f;
	float spec_f_grazing = (1 - specular * 0.08f) * powf(1.f - c_td, 5.f);
	float met_f = specular;

	float alpha_g = (0.5f * roughness / 2.f) * (0.5f + roughness / 2.f);
	float ch_g = c_td > 0.f ? 1.f : 0.f;
	float ti = sqrtf(1.f - ci * ci) / ci;

	float cc_g = ch_g * 2.f / (1.f + sqrtf(1.f + .15259f * ti * ti));
	float spec_g = ch_g * 2.f / (1.f + sqrtf(1.f + alpha_g * alpha_g * ti * ti));

	vec3f rd = base_color * fd;

	vec3f rs = spec_f_incident + ones * spec_gtr * spec_f_grazing * spec_g;//{spec_gtr * spec_f * spec_g, spec_gtr * spec_f * spec_g, spec_gtr * spec_f * spec_g};

	vec3f rm = {spec_gtr * met_f * spec_g, spec_gtr * met_f * spec_g, spec_gtr * met_f * spec_g};
	rm = base_color * rm;

	vec3f rcc = {cc_gtr * cc_f * cc_g, cc_gtr * cc_f * cc_g, cc_gtr * cc_f * cc_g};

	return rd * (1.f - metallic) + (rs * (1.f - metallic) + rm * metallic + rcc) / (4.f * ci * co);
}
