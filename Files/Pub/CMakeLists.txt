cmake_minimum_required(VERSION 2.8)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin") 
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
SET(OpenCV_DIR /home/aminh7325/EmbSys/OpenCV-20220619/OpenCV/installation/OpenCV-master/lib/cmake/opencv4)
project( Publisher )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( Publisher  src/Publisher.cpp )
target_link_libraries( Publisher ${OpenCV_LIBS} -lpaho-mqtt3c -lm -lasound)



INSTALL(TARGETS Publisher DESTINATION bin)
