# ok_raytracer

A path tracer using Embree which is slightly better than mediocre_raytracer.  
Supports:  
* Multithreading via Intel TBB
* The WDAS principled BRDF (Burley et al 2012)
* Importance sampling
* A form of adaptive sampling, inspired by Hyperion's (Burley et al 2018)

Run with:  
 
embree_test <filename> [num_passes] [num_bounces] [silent], e.g. embree_test models/teapot.obj 4 3
