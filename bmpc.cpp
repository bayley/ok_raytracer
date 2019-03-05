#include <stdlib.h>

#include "bmpc.h"
#include "bmp.h"

BMPC::BMPC(int w, int h) {
	width = w;
	height = h;
	red = (unsigned char*)malloc(width*height);
	green = (unsigned char*)malloc(width*height);
	blue = (unsigned char*)malloc(width*height);
}

void BMPC::set_px(int u, int v, unsigned char r, unsigned char g, unsigned char b) {
	red[u * height + v] = r;
  green[u * height + v] = g;
  blue[u * height + v] = b;
}

void BMPC::set_px(int u, int v, float r, float g, float b) {
	if (r > 1.f) r = 1.f; if (g > 1.f) g = 1.f; if (b > 1.f) b = 1.f;
	set_px(u, v, (unsigned char)(r * 255.f), (unsigned char)(g * 255.f), (unsigned char)(b * 255.f));
}

void BMPC::write(char * fname) {
	write_bmp(red, green, blue, width, height, fname);
}
