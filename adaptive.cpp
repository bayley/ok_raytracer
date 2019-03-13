#include <stdio.h>
#include <math.h>

#include "adaptive.h"
#include "hdri.h"
#include "geom.h"

void blur(float * buf, int w, int h, int kern) {
  float * tmp = (float *)malloc(w * h * sizeof(float));
  for (int i = 0; i < w * h; i++) tmp[i] = 0.f;
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      for (int r = 0; r < kern; r++) {
        for (int c = 0; c < kern; c++) {
          float u = row + r < h && col + c < w ? buf[(row + r) * w + col + c] : 0.f;
          tmp[row * w + col] += u;
        }
      }
      tmp[row * w + col] /= (float) (kern * kern);
    }
  }

  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      buf[row * w + col] = tmp[row * w + col];
    }
  }
}

void adapt_counts(HDRI * output, int * counts, int avg) {
  int w = output->width, h = output->height;
  float * var_n = (float*)malloc(w * h * sizeof(float));

  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      vec3f v = output->var(row, col);
      vec3f p = output->at(row, col);
      var_n[row * w + col] = v.x + v.y + v.z;
    }
  }
  blur(var_n, w, h, 4);

  float v_avg = 0.f;
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      v_avg += var_n[row * w + col];
    }
  }
  v_avg /= (float)(w * h);

  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      counts[row * w + col] = (int)((float)(avg) * var_n[row * w + col] / v_avg);
    }
  }
}

