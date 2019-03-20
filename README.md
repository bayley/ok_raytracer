# ok_raytracer

A path tracer using Embree which is slightly better than mediocre_raytracer. Supports the 'Disney BRDF' and multithreading via TBB. Run with:
embree_test <filename> [num_passes] [num_bounces] [silent], e.g. embree_test models/teapot.obj 4 3
