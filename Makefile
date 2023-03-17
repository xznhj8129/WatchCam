CXX = g++
CXXFLAGS = -Wall -Ofast -I/usr/local/include/opencv4
LDLIBS = -lconfig++ -lfmt -lopencv_video -lopencv_text -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_core -lopencv_imgproc -lopencv_dnn

debug: CXXFLAGS += -DDEBUG -g
debug: CCFLAGS += -DDEBUG -g
debug: executable

all: watchcam

executable: watchcam.cpp
	$(CXX) $(CXXFLAGS) -o watchcam watchcam.cpp $(LDLIBS)

clean:
	rm -f test
