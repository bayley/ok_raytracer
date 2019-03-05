#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void write_bmp(unsigned char *red, unsigned char *green, unsigned char *blue,
			   int w, int h, char *fname) {
	FILE *f;
	unsigned char *img = NULL;
	int filesize = 54 + 3 * w * h;  //w is your image width, h is image height, both int
	if (img)
		free(img);
	img = (unsigned char*) malloc(3 * w * h);
	memset(img, 0, sizeof(img));

	for(int i = 0; i < w; i++)
	{
		for(int j = 0; j < h; j++)
		{
			int x = i; int y = h - 1 - j;
			unsigned char r = red[i + j * w];
			unsigned char g = green[i + j * w];
			unsigned char b = blue[i + j * w];
			img[(x + y * w) * 3 + 2] = r;
			img[(x + y * w) * 3 + 1] = g;
			img[(x + y * w) * 3] = b;
		}
	}

	unsigned char bmpfileheader[14] = {'B','M',0,0,0,0,0,0,0,0,54,0,0,0};
	unsigned char bmpinfoheader[40] = {40,0,0,0,0,0,0,0,0,0,0,0,1,0,24,0};
	unsigned char bmppad[3] = {0,0,0};

	bmpfileheader[2] = (unsigned char) filesize;
	bmpfileheader[3] = (unsigned char) (filesize >> 8);
	bmpfileheader[4] = (unsigned char) (filesize >> 16);
	bmpfileheader[5] = (unsigned char) (filesize >> 24);

	bmpinfoheader[4] = (unsigned char) w;
	bmpinfoheader[5] = (unsigned char)(w >> 8);
	bmpinfoheader[6] = (unsigned char)(w >> 16);
	bmpinfoheader[7] = (unsigned char)(w >> 24);

	bmpinfoheader[8] = (unsigned char) h;
	bmpinfoheader[9] = (unsigned char) (h >> 8);
	bmpinfoheader[10] = (unsigned char)(h >> 16);
	bmpinfoheader[11] = (unsigned char)(h >> 24);

	f = fopen(fname,"wb");
	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);
	for(int i = 0; i < h; i++)
	{
		fwrite(img+(w * (h - i - 1) * 3), 3, w, f);
		fwrite(bmppad, 1, (4 - (w * 3) % 4) % 4, f);
	}
	fclose(f);
}

void write_bmp_gs(unsigned char *data,
			   int w, int h, char *fname) {
	FILE *f;
	unsigned char *img = NULL;
	int filesize = 54 + 3 * w * h;  //w is your image width, h is image height, both int
	if (img)
		free(img);
	img = (unsigned char*) malloc(3 * w * h);
	memset(img, 0, sizeof(img));

	for(int i = 0; i < w; i++)
	{
		for(int j = 0; j < h; j++)
		{
			int x = i; int y = h - 1 - j;
			unsigned char r = data[i + j * w];
			unsigned char g = data[i + j * w];
			unsigned char b = data[i + j * w];
			img[(x + y * w) * 3 + 2] = r;
			img[(x + y * w) * 3 + 1] = g;
			img[(x + y * w) * 3] = b;
		}
	}

	unsigned char bmpfileheader[14] = {'B','M',0,0,0,0,0,0,0,0,54,0,0,0};
	unsigned char bmpinfoheader[40] = {40,0,0,0,0,0,0,0,0,0,0,0,1,0,24,0};
	unsigned char bmppad[3] = {0,0,0};

	bmpfileheader[2] = (unsigned char) filesize;
	bmpfileheader[3] = (unsigned char) (filesize >> 8);
	bmpfileheader[4] = (unsigned char) (filesize >> 16);
	bmpfileheader[5] = (unsigned char) (filesize >> 24);

	bmpinfoheader[4] = (unsigned char) w;
	bmpinfoheader[5] = (unsigned char)(w >> 8);
	bmpinfoheader[6] = (unsigned char)(w >> 16);
	bmpinfoheader[7] = (unsigned char)(w >> 24);

	bmpinfoheader[8] = (unsigned char) h;
	bmpinfoheader[9] = (unsigned char) (h >> 8);
	bmpinfoheader[10] = (unsigned char)(h >> 16);
	bmpinfoheader[11] = (unsigned char)(h >> 24);

	f = fopen(fname,"wb");
	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);
	for(int i = 0; i < h; i++)
	{
		fwrite(img+(w * (h - i - 1) * 3), 3, w, f);
		fwrite(bmppad, 1, (4 - (w * 3) % 4) % 4, f);
	}
	fclose(f);
}

void read_bmp(unsigned char *red, unsigned char *green, unsigned char *blue,
						int w, int h, char *fname) {
	FILE *f;
	f = fopen(fname, "rb");

	unsigned char *img = (unsigned char*) malloc(3 * w * h);
	int sz = fread(img, sizeof(unsigned char), 54, f);
	sz = fread(img, sizeof(unsigned char), 3 * w * h, f);
	for (int i = 0; i < 3 * w * h; i += 3) {
		int k = i / 3;
		red[k] = img[i + 2];
		green[k] = img[i + 1];
		blue[k] = img[i];
	}
	fclose(f);
}
