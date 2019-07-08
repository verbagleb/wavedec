SOURCES_CPP=main.cpp cImageYCbCr.cpp cImageRGB.cpp decTree.cpp userLib.cpp cFilter.cpp
SOURCES_C=
FLAGS=-fpermissive -g -fdiagnostics-color=always \
      -DBAND_IMAGE_OUTPUT \
      -DRESTORED_IMAGE_OUTPUT \
      # \
      #
LFLAGS=`gsl-config --cflags --libs`
CPPFLAGS=
CFLAGS=`pkg-config --libs gsl`
#HEADERS=$(SOURCES_CPP:.cpp=.h) $(SOURCES_C:.c=.h) define.h
OBJECTS=$(SOURCES_CPP:.cpp=.o) $(SOURCES_C:.c=.o)
TARGET=wavedec_parametric

all: $(TARGET)

$(TARGET): $(OBJECTS)
	g++ $(FLAGS) $^ -o $@ $(LFLAGS)

.cpp.o:
	g++ $(FLAGS) -c $^ $(CPPFLAGS) 

.c.o:
	g++ $(FLAGS) $(CFLAGS) -c $^

clean: 
	rm -rf $(OBJECTS) $(TARGET)

cleanall: clean
	rm -rf ./output
	rm -rf ./log
