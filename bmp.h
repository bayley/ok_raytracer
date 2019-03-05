#ifndef __BMP_H
#define __BMP_H

void write_bmp(unsigned char *red, unsigned char *green, unsigned char *blue,
			   int w, int h, char *fname);

void write_bmp_gs(unsigned char* data, int w, int h, char *fname);

void read_bmp(unsigned char *red, unsigned char *green, unsigned char *blue,
						int w, int h, char *fname);

#endif
