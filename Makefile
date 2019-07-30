#Linux: linux 	#Windows: win64/win32
PLATFORM=linux
#Linux: w64/w32
BUILD_PLATFORM=w64

SOURCES_CPP=main.cpp cImageYCbCr.cpp cImageRGB.cpp decTree.cpp userLib.cpp cFilter.cpp
SOURCES_C=
FLAGS= -O3 -fdiagnostics-color=always \
      -DBAND_IMAGE_OUTPUT \
      -DRESTORED_IMAGE_OUTPUT \
	  -DOUT_BMP \
	  -DBAND_FILE \
      # \
	  -DPRINT_SEPARATE_BANDS \
	  -DOUT_JPEG \
	  -DOUT_PNG \
      #
#LFLAGS=#`gsl-config --cflags --libs`
LFLAGS+= -ljpeg
LFLAGS+= -lpng -lz
CPPFLAGS= 
CFLAGS=#`pkg-config --libs gsl` 
#HEADERS=$(SOURCES_CPP:.cpp=.h) $(SOURCES_C:.c=.h) define.h
OBJECTS=$(SOURCES_CPP:.cpp=.o) $(SOURCES_C:.c=.o)

ifeq ($(PLATFORM), linux)
	CMPL=g++
	TARGET=wavedec
endif
ifeq ($(PLATFORM), win32)
	CMPL=i686-$(BUILD_PLATFORM)-mingw32-g++ # windows x32
	TARGET=wavedec_x32.exe
	FLAGS+=-DWINDOWS -static-libgcc -static-libstdc++
endif
ifeq ($(PLATFORM), win64)
	CMPL=x86_64-$(BUILD_PLATFORM)-mingw32-g++ # windows x64
	TARGET=wavedec_x64.exe
	FLAGS+=-DWINDOWS -static-libgcc -static-libstdc++
endif

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CMPL) $(FLAGS) $^ -o $@ $(LFLAGS) 

.cpp.o:
	$(CMPL) $(FLAGS) -c $^ $(CPPFLAGS) 

.c.o:
	$(CMPL) $(FLAGS) $(CFLAGS) -c $^

clean: 
	rm -rf $(OBJECTS) wavedec*

cleanall: clean
	rm -rf ./output
	rm -rf ./log
