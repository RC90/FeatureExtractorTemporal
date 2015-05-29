
# on the cluster it's g++ 4.7.0
CC := g++
CFLAGS := -Wall -O3

SOURCES  := main.cpp LBP.cpp gabor.cpp tinyxml2.cpp
OUTPUT   := FeatureExtractorTemporal

OPENCV_LIBS_PATH_CLUSTER := /cm/shared/apps/opencv-2.4.10/gcc/lib
OPENCV_LIBS_PATH_LOCAL := /Users/timur/Documents/OpenCV/lib

BOOST_LIBS_PATH_CLUSTER := /cm/shared/apps/boost_1_57_0/gcc/lib
BOOST_LIBS_PATH_LOCAL := /Users/timur/Documents/Boost/boost_1_57_0/stage/lib

OPENCV_INCLUDES_PATH_CLUSTER := /cm/shared/apps/opencv-2.4.10/gcc/include
OPENCV_INCLUDES_PATH_LOCAL := /Users/timur/Documents/OpenCV/include

BOOST_INCLUDES_PATH_CLUSTER := /cm/shared/apps/boost_1_57_0/gcc/include
BOOST_INCLUDES_PATH_LOCAL := /Users/timur/Documents/Boost/boost_1_57_0

local:
	$(CC) $(CFLAGS) $(SOURCES) -o $(OUTPUT) -I$(OPENCV_INCLUDES_PATH_LOCAL) -I$(BOOST_INCLUDES_PATH_LOCAL) -L$(OPENCV_LIBS_PATH_LOCAL) -L$(BOOST_LIBS_PATH_LOCAL) -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lboost_filesystem -lboost_system

cluster:
	$(CC) $(CFLAGS) $(SOURCES) -o $(OUTPUT) -I$(OPENCV_INCLUDES_PATH_CLUSTER) -I$(BOOST_INCLUDES_PATH_CLUSTER) -L$(OPENCV_LIBS_PATH_CLUSTER) -L$(BOOST_LIBS_PATH_CLUSTER) -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lboost_filesystem

test: 
	$(CC) 			$(SOURCES) -o $(OUTPUT) -I$(OPENCV_INCLUDES_PATH_CLUSTER) -I$(BOOST_INCLUDES_PATH_CLUSTER) -L$(OPENCV_LIBS_PATH_CLUSTER) -L$(BOOST_LIBS_PATH_CLUSTER) -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lboost_filesystem