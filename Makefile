#USE_MOD : Don't use!!! Not work right now - only with initial unsigned chars values!!!
#PRINT_TEST_FILES : Use only with too small images (bitmap_0.bmp, etc.) - too many files are created.

SOURCES_CPP=WaveletDecomposition.cpp cImageYCbCr.cpp cImageRGB.cpp decTree.cpp userLib.cpp cFilter.cpp
SOURCES_C=intra.c
FLAGS=-fpermissive -g -fdiagnostics-color=always \
      -DBAND_IMAGE_OUTPUT \
      # \
      -DINTRA_PRED \
      -DPRINT_TEST_FILES \
      -DSKIP_COLORS \
      -DRESTORED_IMAGE_OUTPUT \
      -DSKIP_ANALYSIS \
      #
LFLAGS=`gsl-config --cflags --libs`
CPPFLAGS=
CFLAGS=`pkg-config --libs gsl`
#HEADERS=$(SOURCES_CPP:.cpp=.h) $(SOURCES_C:.c=.h) define.h
OBJECTS=$(SOURCES_CPP:.cpp=.o) $(SOURCES_C:.c=.o)
TARGET=wavedec_var

all: $(TARGET)

$(TARGET): $(OBJECTS)
	g++ $(FLAGS) $^ -o $@ $(LFLAGS)

.cpp.o:
	g++ $(FLAGS) -c $^ $(CPPFLAGS) 

.c.o:
	g++ $(FLAGS) $(CFLAGS) -c $^

clean: 
	rm -rf $(OBJECTS) $(TARGET)
