#ifndef __OBJ_LOADER_H
#define __OBJ_LOADER_H

#include <embree3/rtcore.h>

int load_obj(RTCDevice * device, RTCScene * scene, char * fname, int id);

#endif
