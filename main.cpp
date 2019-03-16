#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <time.h>

#include <embree3/rtcore.h>
#include <SDL2/SDL.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

#include "obj_loader.h"
#include "geom.h"
#include "ray_utils.h"
#include "hdri.h"
#include "brdf.h"
#include "buffers.h"
#include "adaptive.h"

using namespace tbb;

PrincipledBRDF brdf_objs[64];
HDRI backdrop;
SDL_Window * window; SDL_Renderer * renderer;

/*----preview window----*/
void init_preview(RenderBuffer * b) {
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("Render Output", 
														 SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
										 				 b->width, b->height, SDL_WINDOW_OPENGL);
	renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
}

void update_preview(RenderBuffer * buf, int row_start, int col_start, int row_size, int col_size) {
	for (int x = col_start; x < col_start + col_size; x++) {
		for (int y = row_start; y < row_start + row_size; y++) {
			vec3f p = buf->average[y][x];
			int r, g, b;
			r = (int)(255.f * powf(fminf(p.x, 1.f), 1.f / 2.2f));
			g = (int)(255.f * powf(fminf(p.y, 1.f), 1.f / 2.2f));
			b = (int)(255.f * powf(fminf(p.z, 1.f), 1.f / 2.2f));
			SDL_SetRenderDrawColor(renderer, r, g, b, 0);
			SDL_RenderDrawPoint(renderer, x, buf->height - 1 - y);
		}
	}
	SDL_RenderPresent(renderer);
}

/*----radiance computation----*/
vec3f sample_backdrop(RTCRayHit * rh, float radius) {
  vec3f uv = intersect_backdrop(rh, radius);
  int row = (int)(backdrop.height * uv.y);
  int col = (int)(backdrop.width * uv.x);
  return backdrop[row][col];
}

vec3f gather_radiance(RTCScene * scene, RTCRayHit * rh, RTCIntersectContext * context, int depth, int limit) {
	rtcIntersect1(*scene, context, rh);
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

/*----rendering and threading----*/

void render_tile(RTCScene * scene, Camera * cam, RenderBuffer * output, IBuffer * sample_count, int limit, int row_start, int col_start, int size) {
  RTCRayHit rh;
  RTCIntersectContext context;
  rtcInitIntersectContext(&context);

	vec3f hit, view, light, normal, half;
	int id, side = 0;
	for (int row = row_start; row < row_start + size; row++) {
		for (int col = col_start; col < col_start + size; col++) {
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

class apply_render_tile {
	RTCScene * __scene; Camera * __cam; RenderBuffer * __output; IBuffer * __sample_count;
	int __limit, __tile_size;
public:
	void operator() (const blocked_range<int> &r) const {
		int tile_w = __output->width / __tile_size;
		for (int tile = r.begin(); tile < r.end(); tile++) {
			int row_start = __tile_size * (tile / tile_w);
			int col_start = __tile_size * (tile % tile_w);
			render_tile(__scene, __cam, __output, __sample_count, __limit, row_start, col_start,  __tile_size);
		}
	}
	apply_render_tile(RTCScene * scene, Camera * cam, RenderBuffer * output, IBuffer * sample_count, int limit, int tile_size) {
		__scene = scene; __cam = cam; __output = output; __sample_count = sample_count; __limit = limit; __tile_size = tile_size;
	}
};

int render_pass(RTCScene * scene, Camera * cam, RenderBuffer * output, IBuffer * sample_count, int limit, int tile_size, bool display_preview = false) {
	SDL_Event event;
	int tile_w = output->width / tile_size, tile_h = output->height / tile_size;
	int tile_count = tile_w * tile_h;
	for (int tile = 0; tile < tile_count; tile++) {
		int row_start = tile_size * (tile / tile_w);
		int col_start = tile_size * (tile % tile_w);
		render_tile(scene, cam, output, sample_count, 3, row_start, col_start, tile_size);
		if (display_preview) {
			update_preview(output, row_start, col_start, tile_size, tile_size);
			SDL_PollEvent(&event);
			if (event.type == SDL_QUIT) {
				SDL_Quit();
				return -1;
			}
		}
	}
	return 1;
}

int render_pass_tbb(RTCScene * scene, Camera * cam, RenderBuffer * output, IBuffer * sample_count, int limit, int tile_size, bool display_preview = false) {
	int tile_w = output->width / tile_size, tile_h = output->height / tile_size;
	int tile_count = tile_w * tile_h;
	parallel_for(blocked_range<int>(0, tile_count), apply_render_tile(scene, cam, output, sample_count, limit, tile_size));
	return 1;
}

int main(int argc, char** argv) {
	/*----parse arguments, setup----*/
  if (argc < 2) {
    printf("Usage: embree_test <filename> [n_samples]\n");
    return -1;
  }

	int n_passes = 0;
  if (argc > 2) n_passes = atoi(argv[2]);

	int depth = 3;
	if (argc > 3) depth = atoi(argv[3]);

	bool display_preview = true;
	if (argc > 4) {
		if (strcmp(argv[4], "silent") == 0) display_preview = false;
	}

	printf("Rendering image with %d average spp\n", 16 * (1 << n_passes));

  srand(time(0));
	setbuf(stdout, NULL);

	task_scheduler_init tbb_init;

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
  brdf_objs[0].metallic = 0.0f;
  brdf_objs[0].specular = 1.0f;
  brdf_objs[0].speculartint = 0.f;
  brdf_objs[0].roughness = 1.0f;
  brdf_objs[0].anisotropic = 0.f;
  brdf_objs[0].sheen = 0.f;
  brdf_objs[0].sheentint = 0.f;
  brdf_objs[0].clearcoat = 0.0f;
  brdf_objs[0].clearcoatgloss = 0.0f;
  brdf_objs[0].base_color = {65.f / 255.f, 105.f / 255.f, 225.f / 255.f};

	printf("Building BVH...\n");
	rtcCommitScene(scene);

	/*----set up camera----*/
  RenderBuffer output(2048, 1088);

  Camera cam;
  cam.move(5.f, 8.f, 2.f);
  cam.point(-1.f, -1.5f, 0.f);
  cam.zoom(1.2f);
  cam.resize(output.width, output.height);

	/*----set up preview----*/
	if (display_preview) init_preview(&output);

	/*----first pass----*/
	printf("Initializng adaptive sampler...\n");
	IBuffer sample_count(output.width, output.height);
	for (int row = 0; row < output.height; row++) {
		for (int col = 0; col < output.width; col++) {
			sample_count[row][col] = 16;
		}
	}
	if (render_pass_tbb(&scene, &cam, &output, &sample_count, depth, 32, display_preview) < 0) {
		printf("Killed, no output written\n");
		return -1;
	}
	if (display_preview) update_preview(&output, 0, 0, output.height, output.width);

	/*----render----*/
	printf("Rendering...\n");
	int n_samples = 16;
	for (int pass = 0; pass < n_passes; pass++) {
		adapt_samples(&output, &sample_count, n_samples);
		if (render_pass_tbb(&scene, &cam, &output, &sample_count, depth, 32, display_preview) < 0) {
			printf("Killed, no output written\n");
			return -1;
		}
		if (display_preview) update_preview(&output, 0, 0, output.height, output.width);
		n_samples += n_samples;
	}
	printf("Done!\n");

	/*----write output----*/
	sample_count.write((char*)"samples.bmp", 1);
	output.average.write((char*)"out.bmp", 256.f, 2.2f);
	output.variance.write((char*)"variance.bmp", 256.f);

	/*----wait for user to exit----*/
	if (display_preview) {
		SDL_Event event;
		for (;;) {
			SDL_PollEvent(&event);
			if (event.type == SDL_QUIT) {
				SDL_Quit();		
				break;
			}
		}
	}

	/*----clean up SDL----*/
	if (display_preview) SDL_Quit();

	return 1;
}

