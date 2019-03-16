CXX=g++
CPPFLAGS=-O3 -I.
DEPS = geom.h bmp.h brdf.h obj_loader.h hdri.h buffers.h adaptive.h sampling.h
OBJ = main.o bmp.o geom.o brdf.o obj_loader.o hdri.o buffers.o adaptive.o
LIBS = -lm -lembree3 -lSDL2 -ltbb

%.o: %.c $(DEPS)
	$(CXX) -c -o $@ $< $(CPPFLAGS)

embree_test: $(OBJ)
	$(CXX) -o $@ $^ $(CPPFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o embree_test
