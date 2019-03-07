#include <math.h>

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

vec3f PrincipledBRDF::sample(float ci, float co, float th, float td, float ph, float pd) {
	vec3f ones = {1.f, 1.f, 1.f};

	//diffuse
	float c_td = cosf(td);
	float fd90 = .5f + 2.f * c_td * c_td * roughness;
	float fd = 1.f / M_PI * (1.f + (fd90 - 1.f) * powf(1 - co, 5.f)) * (1.f + (fd90 - 1.f) * powf(1 - ci, 5.f));

	//specular D
	float alpha_spec = roughness * roughness;	
	float alpha_cc = (1.f - clearcoatgloss) * (1.f - clearcoatgloss);

	float c_th = cosf(th);
	float tmp_spec = (alpha_spec * alpha_spec - 1.f) * c_th * c_th + 1.f;
	float tmp_cc = (alpha_cc * alpha_cc - 1.f) * c_th * c_th + 1.f;

	float spec_gtr_norm = alpha_spec * alpha_spec / M_PI;
	float cc_gtr_norm = (alpha_cc * alpha_cc - 1.f) / (M_PI * 2.f * log(alpha_cc));

	float spec_gtr = spec_gtr_norm / (tmp_spec * tmp_spec);
	float cc_gtr = 0.25f * clearcoat * cc_gtr_norm / tmp_cc;

	//specular F
	vec3f spec_f_incident = (ones * (1.f - speculartint) + base_color * speculartint) * specular * 0.08f;
	float spec_f_grazing = (1 - specular * 0.08f) * powf(1.f - c_td, 5.f);
	float cc_f = .04f + 0.96f * powf(1.f - c_td, 5.f);

	//specular G
	float alpha_g_spec = (0.5f + roughness / 2.f) * (0.5f + roughness / 2.f);
	float alpha_g_cc = (0.5f + 0.25f / 2.f) * (0.5f + 0.25f / 2.f);
	float ch_g = c_td > 0.f ? 1.f : 0.f;
	float ti = sqrtf(1.f - ci * ci) / ci; //tan(theta_i)

	float spec_g = ch_g * 2.f / (1.f + sqrtf(1.f + alpha_g_spec * alpha_g_spec * ti * ti));
	float cc_g = ch_g * 2.f / (1.f + sqrtf(1.f + alpha_g_cc * alpha_g_cc * ti * ti));

	//add them together, interpolating between dielectric and metallic
	vec3f rd = base_color * fd;
	vec3f rs = (spec_f_incident + ones * spec_f_grazing) * spec_gtr * spec_g;
	vec3f rm = base_color * specular * spec_gtr * spec_g;
	vec3f rcc = ones * cc_f * cc_gtr * cc_g;

	return rd * (1.f - metallic) + (rs * (1.f - metallic) + rm * metallic + rcc) / (4.f * ci * co);
}
