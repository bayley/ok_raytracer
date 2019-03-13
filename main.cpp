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
#include "adaptive.h"

PrincipledBRDF brdf_objs[64];
HDRI * backdrop;

vec3f sample_backdrop(RTCRayHit * rh, float radius) {
  vec3f uv = intersect_backdrop(rh, radius);
  int row = (int)(backdrop->height * uv.y);
  int col = (int)(backdrop->width * uv.x);
  return backdrop->at(row, col);
}

void render_pass(RTCScene scene, Camera * cam, HDRI * output, int * count) {
  RTCRayHit rh;
  RTCIntersectContext context;
  rtcInitIntersectContext(&context);

	vec3f hit, view, light, normal, half;
	int id, side = 0;
	for (int row = 0; row < output->height; row++) {
		for (int col = 0; col < output->width; col++) {
			int n_samples = count[row * output->width + col];
			if (n_samples == 0) continue;

			rh.ray.tnear = 0.01f; rh.ray.tfar = FLT_MAX;
			rh.hit.instID[0] = -1; rh.hit.geomID = -1;

			set_org(&rh, cam->eye);
			set_dir(&rh, cam->lookat((float)col + randf(), (float)row + randf()));

			rtcIntersect1(scene, &context, &rh);	
			id = rh.hit.geomID;

			if (id == -1) {
				vec3f emit = sample_backdrop(&rh, 25.f);
				output->add(row, col, emit, n_samples);
				continue;
			}

			hit = eval_ray(&rh, rh.ray.tfar);
			view = {-rh.ray.dir_x, -rh.ray.dir_y, -rh.ray.dir_z};
			normal = {rh.hit.Ng_x, rh.hit.Ng_y, rh.hit.Ng_z};
			
			view.normalize(); normal.normalize();
			
			float cos_i = normal.dot(view);
			side = 0; if (cos_i < 0.f) {normal *= -1.f; side = 1; cos_i = -cos_i;}

			/*----begin lighting calculation----*/
			for (int sample = 0; sample < n_samples; sample++) {
				float roulette = randf();
	
				float pdf;
				float r_specular = 0.5f + 0.5f * brdf_objs[id].metallic;
				float r_diffuse = 1.f - r_specular;
				
				if (roulette < r_specular) {
					light = random_specular(&pdf, brdf_objs[id].roughness, view, normal);
				} else if (roulette < r_specular + r_diffuse) {
					light = random_diffuse(&pdf, normal);
				}
				half = view + light;
				half.normalize();

				float cos_o = normal.dot(light);
				if (cos_o < 0.f) {
					output->add(row, col, {0.f, 0.f, 0.f});
					continue;
				}

				float cos_th = normal.dot(half);
				float cos_td = light.dot(half);

				vec3f shade;
				if (roulette < r_specular) {
					shade = brdf_objs[id].sample_specular(cos_i, cos_o, cos_th, cos_td) / r_specular;
				} else if (roulette < r_specular + r_diffuse) {
					shade = brdf_objs[id].sample_diffuse(cos_i, cos_o, cos_th, cos_td) / r_diffuse;
				}

				rh.ray.tnear = 0.01f; rh.ray.tfar = FLT_MAX;
				rh.hit.instID[0] = -1; rh.hit.geomID = -1;

				set_org(&rh, hit);
				set_dir(&rh, light);
			
				rtcIntersect1(scene, &context, &rh);

				if (rh.hit.geomID == -1) {
					vec3f emit = sample_backdrop(&rh, 25.f);
					output->add(row, col, shade * emit * cos_o / pdf);
				} else {
					output->add(row, col, {0.f, 0.f, 0.f});
				}
			}
			/*----end lighting calculation----*/
		}
	}
}

int main(int argc, char** argv) {
	/*----parse arguments, setup----*/
  if (argc < 2) {
    printf("Usage: embree_test <filename> [n_aa] [n_samples]\n");
    return -1;
  }

	int n_aa = 1, n_passes = 0;
  if (argc > 2) n_aa = atoi(argv[2]);
	if (argc > 3) n_passes = atoi(argv[3]);

  srand(time(0));
	setbuf(stdout, NULL);

	/*----load scene----*/
  RTCDevice device = rtcNewDevice("");
  RTCScene scene = rtcNewScene(device);

	backdrop = new HDRI(2048, 1024);
	if (backdrop->load((char*)"textures/studio.bmp") < 0) {
		printf("Backdrop not found\n");
		return -1;
	}

	if (load_obj(device, scene, argv[1], 0) < 0) {
    printf("%s not found\n", argv[1]);
    return -1;
  }

	brdf_objs[0].subsurface = 0.f;
  brdf_objs[0].metallic = 0.0f;
  brdf_objs[0].specular = 1.0f;
  brdf_objs[0].speculartint = 0.f;
  brdf_objs[0].roughness = 0.3f;
  brdf_objs[0].anisotropic = 0.f;
  brdf_objs[0].sheen = 0.f;
  brdf_objs[0].sheentint = 0.f;
  brdf_objs[0].clearcoat = 0.0f;
  brdf_objs[0].clearcoatgloss = 0.0f;
  brdf_objs[0].base_color = {65.f / 255.f, 105.f / 255.f, 225.f / 255.f};

	printf("Creating BVH...\n");
	rtcCommitScene(scene);

	/*----set up camera----*/
  HDRI output(1920, 1080, true);

  Camera cam;
  cam.move(5.f, 8.f, 2.f);
  cam.point(-1.f, -1.5f, 0.f);
  cam.zoom(1.2f);
  cam.resize(output.width, output.height);

	/*----initialize adaptive sampling----*/
	printf("Initializing adaptive sampler with %d samples: ", n_aa);
	int * sample_counts = (int*)malloc(output.width * output.height * sizeof(int));
	for (int i = 0; i < output.width * output.height; i++) sample_counts[i] = 1;
	for (int aa = 0; aa < n_aa; aa++) {
		printf("*");
		render_pass(scene, &cam, &output, sample_counts);
	}
	printf("\n");

	/*----render----*/
	printf("Rendering: ");
	for (int aa = 0; aa < n_aa; aa++) {
		int n_samples = 1;
		for (int pass = 0; pass < n_passes; pass++) {
			adapt_counts(&output, sample_counts, n_samples);
			render_pass(scene, &cam, &output, sample_counts);
			n_samples += n_samples;
		}
		printf("*");
	}
	printf("\n"); 

	/*----write output----*/
	output.write_avg((char*)"out.bmp");
	output.write_var((char*)"variance.bmp", 8.f);
}

