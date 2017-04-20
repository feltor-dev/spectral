INCLUDE = -I$(HOME)/include
CXX = g++
CFLAGS = -Wall -fopenmp -std=c++0x
LIBS = -lfftw3 -lm
DEBUG = 

GLFLAGS=$$(pkg-config --static --libs glfw3)

spectral: spectral_b.cpp
	$(CXX) $(DEBUG) -O3 $< $(CFLAGS) $(INCLUDE) $(LIBS) $(GLFLAGS) -o $@
	./$@

spectral_t: spectral_t.cpp
	$(CXX) -DTL_DEBUG $< $(CFLAGS) $(INCLUDE) $(LIBS) $(GLFLAGS) -o $@
	./$@

spectral_b: spectral_b.cpp
	$(CXX) $(DEBUG) -O3 $< $(CFLAGS) $(INCLUDE) $(LIBS) $(GLFLAGS) -o $@
	./$@

texture_t: texture_t.cpp texture.h
	$(CXX) -DTL_DEBUG $< $(CFLAGS) $(INCLUDE) $(LIBS) $(GLFLAGS) -o $@
	./$@


%_t: %_t.cpp %.h
	$(CXX) -DTL_DEBUG $< $(CFLAGS) $(INCLUDE) $(LIBS) -o $@
	./$@

%_b: %_b.cpp %.h
	$(CXX) -O3  $< $(CFLAGS) $(INCLUDE) $(LIBS) -o $@
	./$@

.PHONY: doc clean

doc:
	doxygen Doxyfile

clean:
	rm -f *_t *_b spectral
