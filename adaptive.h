#ifndef __ADAPTIVE_H
#define __ADAPTIVE_H

#include "buffers.h"
#include "hdri.h"

void adapt_samples(RenderBuffer * output, IBuffer * samples, int n);

#endif
