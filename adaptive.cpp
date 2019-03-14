#include <stdlib.h>
#include <stdio.h>

#include "adaptive.h"
#include "buffers.h"
#include "hdri.h"
#include "geom.h"

void blur (FBuffer * b, int k) {
	//horizontal direction
	float * tmp = (float*)malloc(b->width * sizeof(float));
	for (int row = 0; row < b->height; row++) {
		float s = 0.f;
		for (int i = 0; i < k; i++) {
			s += (*b)[row][k];
		}
		for (int col = 0; col < b->width - k; col++) {
			tmp[col] = s / (float)k;
			s -= (*b)[row][col];
			s += (*b)[row][col + k];
		}
		for (int col = b->width - k; col < b->width; col++) {
			tmp[col] = (*b)[row][col];
		}
		for (int col = 0; col < b->width; col++) (*b)[row][col] = tmp[col];
	}
	free(tmp);
	
	//vertical direction
	tmp = (float*)malloc(b->height * sizeof(float));
	for (int col = 0; col < b->width; col++) {
		float s = 0.f;
		for (int i = 0; i < k; i++) {
			s += (*b)[i][col];
		}
		for (int row = 0; row < b->height - k; row++) {
			tmp[row] = s / (float)k;
			s -= (*b)[row][col];
			s += (*b)[row + k][col];
		}
		for (int row = b->height - k; row < b->height; row++) {
			tmp[row] = (*b)[row][col];
		}
		for (int row = 0; row < b->height; row++) (*b)[row][col] = tmp[row];
	}
	free(tmp);
}

void adapt_samples(RenderBuffer * output, IBuffer * samples, int n) {
	int w = output->width, h = output->height;

	//flatten and normalize variance
	FBuffer var(w, h);
	for (int row = 0; row < h; row++) {
		for (int col = 0; col < w; col++) {
			vec3f v = output->variance[row][col];
			vec3f p = output->average[row][col];
			float n = v.x + v.y + v.z;
			float d = sqrtf(p.x + p.y + p.z);
			if (d == 0.f) var[row][col] = 0.f;
			else var[row][col] = n / d;
		}
	}

	//blur variance
	blur(&var, 10);

	//clip variances, accumulate total
	float var_avg = 0.f;
	for (int row = 0; row < h; row++) {
		for (int col = 0; col < w; col++) {
			var[row][col] = fminf(1.f, var[row][col]);
			var_avg += var[row][col];
		}
	}
	var_avg /= (float)(w * h);

	//map sample count
	for (int row = 0; row < h; row++) {
		for (int col = 0; col < w; col++) {
			int k = (int)(var[row][col] * (float)n / var_avg);
			(*samples)[row][col] = k;
		}
	}
}
