#include <stdio.h>
#include <stdlib.h>
#include <embree3/rtcore.h>
#include <math.h>
#include <float.h>
#include <time.h>

#include "obj_loader.h"
#include "geom.h"
#include "ray_utils.h"
#include "hdri.h"
#include "brdf.h"
#include "buffers.h"
#include "adaptive.h"

PrincipledBRDF brdf_objs[64];
HDRI backdrop;

vec3f sample_backdrop(RTCRayHit * rh, float radius) {
  vec3f uv = intersect_backdrop(rh, radius);
  int row = (int)(backdrop.height * uv.y);
  int col = (int)(backdrop.width * uv.x);
  return backdrop[row][col];
}

vec3f gather_radiance(RTCScene scene, RTCRayHit * rh, RTCIntersectContext * context, int depth, int limit) {
	rtcIntersect1(scene, context, rh);
	int id = rh->hit.geomID;
	
	if (id == -1) {
		return sample_backdrop(rh, 25.f);
	}	

	depth++; if (depth == limit) return vec3f(0.f, 0.f, 0.f);

	vec3f hit = eval_ray(rh, rh->ray.tfar);
	vec3f view = {-rh->ray.dir_x, -rh->ray.dir_y, -rh->ray.dir_z};
	vec3f normal = {rh->hit.Ng_x, rh->hit.Ng_y, rh->hit.Ng_z};

	view.normalize(); normal.normalize();

	float cos_i = normal.dot(view);
	int side = 0; if (cos_i < 0.f) {normal *= -1.f; side = 1; cos_i = -cos_i;}	

	float roulette = randf();

	float pdf;
	float r_specular = 0.5f + 0.5f * brdf_objs[id].metallic;
	float r_diffuse = 1.f - r_specular;

	vec3f light;	
	if (roulette < r_specular) {
		light = random_specular(&pdf, brdf_objs[id].roughness, view, normal);
	} else if (roulette < r_specular + r_diffuse) {
		light = random_diffuse(&pdf, normal);
	}
	vec3f half = view + light;
	half.normalize();

	float cos_o = normal.dot(light);
	if (cos_o < 0.f) {
		return vec3f(0.f, 0.f, 0.f);
	}

	float cos_th = normal.dot(half);
	float cos_td = light.dot(half);

	vec3f shade;
	if (roulette < r_specular) {
		shade = brdf_objs[id].sample_specular(cos_i, cos_o, cos_th, cos_td) / r_specular;
	} else if (roulette < r_specular + r_diffuse) {
		shade = brdf_objs[id].sample_diffuse(cos_i, cos_o, cos_th, cos_td) / r_diffuse;
	}

	rh->ray.tnear = 0.01f; rh->ray.tfar = FLT_MAX;
	rh->hit.instID[0] = -1; rh->hit.geomID = -1;
	
	set_org(rh, hit);
	set_dir(rh, light);

	vec3f bounce = gather_radiance(scene, rh, context, depth, limit);
	return shade * bounce * cos_o / pdf;	
}

void render_pass(RTCScene scene, Camera * cam, RenderBuffer * output, IBuffer * sample_count, int limit) {
  RTCRayHit rh;
  RTCIntersectContext context;
  rtcInitIntersectContext(&context);

	vec3f hit, view, light, normal, half;
	int id, side = 0;
	for (int row = 0; row < output->height; row++) {
		for (int col = 0; col < output->width; col++) {
			int n_samples = (*sample_count)[row][col];
			for (int sample = 0; sample < n_samples; sample++) {
				rh.ray.tnear = 0.01f; rh.ray.tfar = FLT_MAX;
				rh.hit.instID[0] = -1; rh.hit.geomID = -1;
	
				set_org(&rh, cam->eye);
				set_dir(&rh, cam->lookat((float)col + randf(), (float)row + randf()));

				output->add(row, col, gather_radiance(scene, &rh, &context, 0, limit));
			}
		}
	}
}

int main(int argc, char** argv) {
	/*----test code goes here----*/
	/*----parse arguments, setup----*/
  if (argc < 2) {
    printf("Usage: embree_test <filename> [n_aa] [n_samples]\n");
    return -1;
  }

	int n_passes = 0;
  if (argc > 2) n_passes = atoi(argv[2]);

	printf("Rendering image with %d average spp\n", 16 * (1 << n_passes));

  srand(time(0));
	setbuf(stdout, NULL);

	/*----load scene----*/
  RTCDevice device = rtcNewDevice("");
  RTCScene scene = rtcNewScene(device);

	backdrop.initialize(2048, 1024);
	if (backdrop.load((char*)"textures/studio.bmp") < 0) {
		printf("Backdrop not found\n");
		return -1;
	}

	if (load_obj(device, scene, argv[1], 0) < 0) {
    printf("%s not found\n", argv[1]);
    return -1;
  }

	brdf_objs[0].subsurface = 0.f;
  brdf_objs[0].metallic = 1.0f;
  brdf_objs[0].specular = 1.0f;
  brdf_objs[0].speculartint = 0.f;
  brdf_objs[0].roughness = 0.3f;
  brdf_objs[0].anisotropic = 0.f;
  brdf_objs[0].sheen = 0.f;
  brdf_objs[0].sheentint = 0.f;
  brdf_objs[0].clearcoat = 0.0f;
  brdf_objs[0].clearcoatgloss = 0.0f;
  brdf_objs[0].base_color = {183.f / 255.f, 110.f / 255.f, 121.f / 255.f};

	printf("Building BVH...\n");
	rtcCommitScene(scene);

	/*----set up camera----*/
  RenderBuffer output(1920, 1080);

  Camera cam;
  cam.move(5.f, 8.f, 2.f);
  cam.point(-1.f, -1.5f, 0.f);
  cam.zoom(1.2f);
  cam.resize(output.width, output.height);
	cam.resize(1920, 1080);

	/*----first pass----*/
	printf("Initializng adaptive sampler...\n");
	IBuffer sample_count(output.width, output.height);
	for (int row = 0; row < output.height; row++) {
		for (int col = 0; col < output.width; col++) {
			sample_count[row][col] = 16;
		}
	}
	render_pass(scene, &cam, &output, &sample_count, 3);

	/*----render----*/
	printf("Rendering...\n");
	int n_samples = 16;
	for (int pass = 0; pass < n_passes; pass++) {
		adapt_samples(&output, &sample_count, n_samples);
		render_pass(scene, &cam, &output, &sample_count, 3);
		n_samples += n_samples;
	}

	/*----write output----*/
	sample_count.write((char*)"samples.bmp", 1);
	output.average.write((char*)"out.bmp", 256.f, 2.2f);
	output.variance.write((char*)"variance.bmp", 256.f);
}

