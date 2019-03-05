#include <stdlib.h>
#include <stdio.h>
#include <embree3/rtcore.h>

#include "geom.h"
#include "obj_loader.h"

int load_obj(RTCDevice * device, RTCScene * scene, char * fname, int id) {
	RTCGeometry geom = rtcNewGeometry(*device, RTC_GEOMETRY_TYPE_TRIANGLE);
	int num_triangles = 0, num_vertices = 0;

	FILE * in = fopen(fname, "r");
  char * line = NULL; size_t len = 0; ssize_t nread;
  while ((nread = getline(&line, &len, in)) != -1) {
    if (line[0] == 'v') num_vertices++;
    if (line[0] == 'f') num_triangles++;
  }
  free(line);

  Vertex * vertices  = (Vertex*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Vertex), num_vertices);
  Triangle * triangles = (Triangle*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(Triangle), num_triangles);

  line = NULL; len = 0; fseek(in, 0L, SEEK_SET);
  char fn[100]; int v_index = 0, f_index = 0;
  float x, y, z; int u, v, w;
  while ((nread = getline(&line, &len, in)) != -1) {
    if (line[0] == 'v') {
      sscanf(line, "%s %f %f %f", fn, &x, &y, &z);
      vertices[v_index].x = x; vertices[v_index].y = y; vertices[v_index].z = z;
      v_index++;
    } else if (line[0] == 'f') {
      sscanf(line, "%s %d %d %d", fn, &u, &v, &w);
      triangles[f_index].v0 = u - 1; triangles[f_index].v1 = v - 1; triangles[f_index].v2 = w - 1;
      f_index++;
    }
  }

  fclose(in);

  rtcCommitGeometry(geom);
  rtcAttachGeometryByID(*scene, geom, id);
	rtcReleaseGeometry(geom);
}
