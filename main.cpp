#include <stdio.h>
#include <stdlib.h>
#include <embree3/rtcore.h>
#include <math.h>
#include <float.h>
#include <time.h>

#include "obj_loader.h"
#include "geom.h"
#include "ray_utils.h"
#include "bmp.h"
#include "bmpc.h"
#include "brdf.h"

brdf_t brdfs[64];
vec3f base_colors[64];
vec3f * hdri;

vec3f sample_hdri(RTCRayHit * rh, int w, int h, float radius) {
	vec3f uv = intersect_hdri(rh, 25.f);
	int u = (int)(w * uv.x);
	int v = (int)(h * uv.y);
	return hdri[v * w + u];
}

int load_hdri(char * fname, int hdri_w, int hdri_h) {
	unsigned char *h_red, *h_green, *h_blue;

	h_red = (unsigned char *)malloc(hdri_h * hdri_w);
	h_green = (unsigned char *)malloc(hdri_h * hdri_w);
	h_blue = (unsigned char *)malloc(hdri_h * hdri_w);
	hdri = (vec3f *)malloc(hdri_h * hdri_w * sizeof(vec3f));

	if (read_bmp(h_red, h_green, h_blue, hdri_w, hdri_h, fname) < 0) return -1;

	for (int i = 0; i < hdri_w * hdri_h; i++) {
		float r, g, b;
		r = (float)h_red[i] / 255.f; g = (float)h_green[i] / 255.f; b = (float)h_blue[i] / 255.f;
		hdri[i] = {powf(r, 2.2f), powf(g, 2.2f), powf(b, 2.2f)}; //input is sRGB
	}

	free(h_red); free(h_green); free(h_blue);
	return 1;
}

int main(int argc, char ** argv) {
	if (argc < 2) {
		printf("Usage: embree_test <filename> [n_aa] [n_diffuse]\n");
		return -1;
	}

	srand(time(0));

	RTCDevice device = rtcNewDevice("");
	RTCScene scene = rtcNewScene(device);

	int hdri_w = 1500, hdri_h = 750;
	if (load_hdri((char*)"textures/grass.bmp", hdri_w, hdri_h) < 0) {
		printf("HDRI not found\n");
		return -1;
	}

	if (load_obj(device, scene, argv[1], 0) < 0) {
		printf("%s not found\n", argv[1]);
		return -1;
	}
	brdfs[0] = brdf_blinn_phong;//brdf_lambert;
	base_colors[0] = {1.f, 1.f, 1.f};

	rtcCommitScene(scene);

	BMPC output(1920, 1080);

	Camera cam;
	cam.move(5.f, 8.f, 2.f);
	cam.point(-1.f, -1.5f, 0.f);
	cam.zoom(1.2f);
	cam.resize(output.width, output.height);

	int n_aa = 16, n_diffuse = 8;
	if (argc > 2) n_aa = atoi(argv[2]);
	if (argc > 3) n_diffuse = atoi(argv[3]);

	RTCRayHit rh;
	RTCIntersectContext context;
	rtcInitIntersectContext(&context);

	vec3f hit_p, hit_n, last_dir;
	int last_id, side = 0;
	vec3f total, diffuse;

	for (int row = 0; row < output.height; row++) {
		for (int col = 0; col < output.width; col++) {
			total = {0.f, 0.f, 0.f};
			
			for (int aa = 0; aa < n_aa; aa++) {
				rh.ray.tnear = 0.01f; rh.ray.tfar = FLT_MAX;
				rh.hit.instID[0] = -1; rh.hit.geomID = -1;

				set_org(&rh, cam.eye);
				set_dir(&rh, cam.lookat((float)col + randf(), (float)row + randf()));

				rtcIntersect1(scene, &context, &rh);
				last_id = rh.hit.geomID;

				if (last_id == -1) {
					total += sample_hdri(&rh, hdri_w, hdri_h, 25.f); //TODO: optimize me!
					continue;
				} 

				hit_p = eval_ray(&rh, rh.ray.tfar);

				last_dir = {rh.ray.dir_x, rh.ray.dir_y, rh.ray.dir_z};
				last_dir.normalize();

				hit_n = {rh.hit.Ng_x, rh.hit.Ng_y, rh.hit.Ng_z};
				side = 0; if (last_dir.dot(hit_n) > 0.f) {hit_n *= -1.f; side = 1;};
				hit_n.normalize();

				diffuse = {0.f, 0.f, 0.f};

				float cos_g, th, td, ph, pd;
				for (int sample = 0; sample < n_diffuse; sample++) {
					vec3f out_dir = random_dir(hit_n);

					cos_g = hit_n.dot(out_dir);
					get_angles(last_dir, out_dir, hit_n, &th, &td, &ph, &pd); //TODO: optimize me!
					vec3f pdf = brdfs[last_id](th, td, ph, pd);
				
					rh.ray.tnear = 0.01f; rh.ray.tfar = FLT_MAX;
					rh.hit.instID[0] = -1; rh.hit.geomID = -1;

					set_org(&rh, hit_p);
					set_dir(&rh, out_dir);

					rtcIntersect1(scene, &context, &rh);
				
					if (rh.hit.geomID == -1) {
						vec3f emit = sample_hdri(&rh, hdri_w, hdri_h, 25.f); //TODO: optimize me!
						diffuse += emit * base_colors[last_id] * cos_g * pdf;
					}
				}
				diffuse /= (float)n_diffuse;
				total += diffuse;
			}
			total /= (float)n_aa;
			output.set_px(row, col, powf(total.x, 1.f / 2.2), powf(total.y, 1.f / 2.2), powf(total.z, 1.f / 2.2)); //gamma = 2.2
		}
	}

	output.write((char*)"out.bmp");

	rtcReleaseScene(scene);
	rtcReleaseDevice(device);
}
