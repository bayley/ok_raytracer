#ifndef __BMPC_H
#define __BMPC_H

class BMPC {
public:
	BMPC(int w, int h);
public:
	void set_px(int u, int v, unsigned char r, unsigned char g, unsigned char b);
	void set_px(int u, int v, float r, float g, float b);
	void write(char * fname);
public:
	int width, height;
private:
	unsigned char *red, *green, *blue;
};

#endif
