#ifndef __SAMPLING_H
#define __SAMPLING_H

#include "geom.h"
#include "brdf.h"

typedef struct {
	float pdf_s;
	float pdf_d;
	float pdf_c;
} pdf_data;

inline vec3f local_u(vec3f hit_n) { 
  vec3f hit_u = {-hit_n.y, hit_n.x, 0.f};
  if (hit_u.abs() == 0.f) { 
    hit_u = {0.f, -hit_n.z, hit_n.y};
  }
  hit_u.normalize();
  return hit_u;
}

inline float randf() {
  static thread_local std::mt19937 generator;
  std::uniform_real_distribution<float> distribution(0.f, 1.f);
  return fminf(distribution(generator), 1 - FLT_EPSILON);
}

inline float d_diffuse(float cos_theta) {
	return cos_theta / M_PI;
}

inline float d_gtr1(float clearcoatgloss, float cos_theta_h, float l_dot_h) {
	float alpha = lerp(.1f, 0.001f, clearcoatgloss);
	float a2 = alpha * alpha;
	float t = 1.f + (a2 - 1.f) * cos_theta_h * cos_theta_h;
	return (a2 - 1.f) / (M_PI * log(a2)) * 1.f / t * cos_theta_h / (4.f * l_dot_h);
}

inline float d_gtr2(float roughness, float cos_theta_h, float l_dot_h) {
	float alpha = fmaxf(0.001f, roughness * roughness);
	float a2 = alpha * alpha;
	float t = 1.f + (a2 - 1.f) * cos_theta_h * cos_theta_h;
	return a2 / M_PI * 1.f / (t * t) * cos_theta_h / (4.f * l_dot_h);
}

inline vec3f random_diffuse(pdf_data * pdfs, PrincipledBRDF * brdf, vec3f view, vec3f n) {
  vec3f u = local_u(n);
  vec3f v = n.cross(u);

  float e = randf();

  float cos_theta = sqrtf(1.f - e);
  float sin_theta = sqrtf(e);

  float hit_phi = 2.f * M_PI * randf();

  float c_u = sin_theta * cosf(hit_phi);
  float c_v = sin_theta * sinf(hit_phi);

  vec3f out_dir = u * c_u + v * c_v + n * cos_theta;

  vec3f half = view + out_dir;
  half.normalize();

  float cos_theta_h = half.dot(n);
  float l_dot_h = out_dir.dot(half);

	pdfs->pdf_s = d_gtr2(brdf->roughness, cos_theta_h, l_dot_h);
	pdfs->pdf_d = d_diffuse(cos_theta);
	pdfs->pdf_c = d_gtr1(brdf->clearcoatgloss, cos_theta_h, l_dot_h);

  return out_dir;
}

inline vec3f random_specular(pdf_data * pdfs, PrincipledBRDF * brdf, vec3f view, vec3f n) {
  vec3f u = local_u(n);
  vec3f v = n.cross(u);

  float alpha = fmaxf(0.001f, brdf->roughness * brdf->roughness);
  float a2 = alpha * alpha;
  float e = randf();

  float cos_theta_h = sqrtf((1.f - e) / (1 + (a2 - 1.f) * e));
  float sin_theta_h = sqrtf(1.f - cos_theta_h * cos_theta_h);

  float hit_phi = 2.f * M_PI * randf();

  float c_u = sin_theta_h * cosf(hit_phi);
  float c_v = sin_theta_h * sinf(hit_phi);

  vec3f half = u * c_u + v * c_v + n * cos_theta_h;
  vec3f out_dir = half * 2.f * view.dot(half) - view;

  float cos_theta = out_dir.dot(n);
  float l_dot_h = out_dir.dot(half);

	pdfs->pdf_s = d_gtr2(brdf->roughness, cos_theta_h, l_dot_h);
	pdfs->pdf_d = d_diffuse(cos_theta);
	pdfs->pdf_c = d_gtr1(brdf->clearcoatgloss, cos_theta_h, l_dot_h);

  return out_dir;
}

inline vec3f random_clearcoat(pdf_data * pdfs, PrincipledBRDF * brdf, vec3f view, vec3f n) {
  vec3f u = local_u(n);
  vec3f v = n.cross(u);

  float alpha = lerp(.1f, 0.001f, brdf->clearcoatgloss);
  float a2 = alpha * alpha;
  float e = randf();

  float cos_theta_h = sqrtf((1.f - powf(a2, 1.f - e)) / (1.f - a2));
  float sin_theta_h = sqrtf(1.f - cos_theta_h * cos_theta_h);

  float hit_phi = 2.f * M_PI * randf();

  float c_u = sin_theta_h * cosf(hit_phi);
  float c_v = sin_theta_h * sinf(hit_phi);

  vec3f half = u * c_u + v * c_v + n * cos_theta_h;
  vec3f out_dir = half * 2.f * view.dot(half) - view;

  float l_dot_h = out_dir.dot(half);
  float cos_theta = out_dir.dot(n);

	pdfs->pdf_s = d_gtr2(brdf->roughness, cos_theta_h, l_dot_h);
	pdfs->pdf_d = d_diffuse(cos_theta);
	pdfs->pdf_c = d_gtr1(brdf->clearcoatgloss, cos_theta_h, l_dot_h);

  return out_dir;
}

#endif
