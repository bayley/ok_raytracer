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

int main(int argc, char ** argv) {
	srand(time(0));

	RTCDevice device = rtcNewDevice("");
	RTCScene scene = rtcNewScene(device);

	load_obj(device, scene, (char*)"models/teapot.obj", 0);
	brdfs[0] = brdf_lambert;
	base_colors[0] = {1.f, 1.f, 1.f};

	rtcCommitScene(scene);

	int hdri_w = 1440, hdri_h = 1440;
	unsigned char *h_red, *h_green, *h_blue;
	h_red = (unsigned char*)malloc(hdri_h * hdri_w);
	h_green = (unsigned char*)malloc(hdri_h * hdri_w);
	h_blue = (unsigned char*)malloc(hdri_h * hdri_w);
	hdri = (vec3f *)malloc(hdri_h * hdri_w * sizeof(vec3f));
	read_bmp(h_red, h_green, h_blue, hdri_w, hdri_h, (char*)"textures/bliss.bmp");
	for (int i = 0; i < hdri_w * hdri_h; i++) {
		hdri[i] = {(float)h_red[i] / 255.f, (float)h_green[i] / 255.f, (float)h_blue[i] / 255.f};
	}
	free(h_red); free(h_green); free(h_blue);

	BMPC output(1000, 1000);

	Camera cam;
	cam.move(5.f, 8.f, 0.f);
	cam.point(-1.f, -1.5f,0.f);
	cam.zoom(0.8f);
	cam.resize(output.width, output.height);

	RTCRayHit rh;
	RTCIntersectContext context;
	rtcInitIntersectContext(&context);

	vec3f hit_p, hit_n, last_dir;
	int last_id;
	float backside;
	vec3f g_illum;

	for (int row = 0; row < output.width; row++) {
		for (int col = 0; col < output.width; col++) {
			rh.ray.tnear = 0.01f; rh.ray.tfar = FLT_MAX;
			rh.hit.instID[0] = -1; rh.hit.geomID = -1;

			set_org(&rh, cam.eye);
			set_dir(&rh, cam.lookat(row, col));

			rtcIntersect1(scene, &context, &rh);
			last_id = rh.hit.geomID;

			if (last_id == -1) {
				vec3f emit = sample_hdri(&rh, hdri_w, hdri_h, 25.f);
				output.set_px(row, col, emit.x, emit.y, emit.z);
				continue;
			} 

			hit_p = eval_ray(&rh, rh.ray.tfar);
			hit_n = {rh.hit.Ng_x, rh.hit.Ng_y, rh.hit.Ng_z};
			hit_n.normalize();
			last_dir = {rh.ray.dir_x, rh.ray.dir_y, rh.ray.dir_z};
			backside = last_dir.dot(hit_n) > 0.f ? -1.f : 1.f;

			g_illum = {0.f, 0.f, 0.f};

			for (int sample = 0; sample < 128; sample++) {
				vec3f out_dir = random_dir(hit_n, backside);

				float cos_g = backside * hit_n.dot(out_dir);
				float pdf = brdfs[last_id](0.f, 0.f, 0.f, 0.f);
				
				rh.ray.tnear = 0.01f; rh.ray.tfar = FLT_MAX;
				rh.hit.instID[0] = -1; rh.hit.geomID = -1;

				set_org(&rh, hit_p);
				set_dir(&rh, out_dir);

				rtcIntersect1(scene, &context, &rh);
				
				if (rh.hit.geomID == -1) {
					vec3f emit = sample_hdri(&rh, hdri_w, hdri_h, 25.f);
					g_illum += emit * base_colors[last_id] * cos_g * pdf;
				}
			}
			g_illum /= 128.f;
			output.set_px(row, col, g_illum.x, g_illum.y, g_illum.z);
		}
	}

	output.write((char*)"out.bmp");

	rtcReleaseScene(scene);
	rtcReleaseDevice(device);
}
