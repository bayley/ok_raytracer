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

int main(int argc, char ** argv) {
	srand(time(0));

	RTCDevice device = rtcNewDevice("");
	RTCScene scene = rtcNewScene(device);

	load_obj(device, scene, (char*)"models/teapot.obj", 0);
	rtcCommitScene(scene);

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
	vec3f g_illum;
	for (int row = 0; row < output.width; row++) {
		for (int col = 0; col < output.width; col++) {
			rh.ray.tnear = 0.01f; rh.ray.tfar = FLT_MAX;
			rh.hit.instID[0] = -1; rh.hit.geomID = -1;

			set_org(&rh, cam.eye);
			set_dir(&rh, cam.lookat(row, col));

			rtcIntersect1(scene, &context, &rh);

			if (rh.hit.geomID == -1) {
				output.set_px(row, col, 1.f, 1.f, 1.f);
				continue;
			} 

			hit_p = eval_ray(&rh, rh.ray.tfar);
			hit_n = {rh.hit.Ng_x, rh.hit.Ng_y, rh.hit.Ng_z};
			hit_n.normalize();

			g_illum = {0.f, 0.f, 0.f};

			for (int sample = 0; sample < 128; sample++) {
				float backside = last_dir.dot(hit_n) > 0.f ? -1.f : 1.f;
				vec3f out_dir = random_dir(hit_n, backside);
				float cos_g = backside * hit_n.dot(out_dir);

				
				rh.ray.tnear = 0.01f; rh.ray.tfar = FLT_MAX;
				rh.hit.instID[0] = -1; rh.hit.geomID = -1;
				set_org(&rh, hit_p);
				set_dir(&rh, out_dir);

				rtcIntersect1(scene, &context, &rh);
				
				if (rh.hit.geomID == -1) {
					vec3f emit = {1.f, 1.f, 1.f};
					g_illum += emit * cos_g;
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
