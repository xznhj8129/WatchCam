CXX = g++
CXXFLAGS = -Wall -Ofast -I/usr/include/opencv4
LDLIBS = -lconfig++ -lfmt -lopencv_video -lopencv_text -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_core -lopencv_imgproc

all: watchcam

test: watchcam.cpp
	$(CXX) $(CXXFLAGS) -o watchcam watchcam.cpp $(LDLIBS)

clean:
	rm -f test
