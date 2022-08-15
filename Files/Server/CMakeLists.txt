cmake_minimum_required(VERSION 3.3)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin") 
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
SET(OpenCV_DIR /home/aminh7325/EmbSys/OpenCV-20220619/OpenCV/installation/OpenCV-master/lib/cmake/opencv4)
project(Server)
find_package( OpenCV REQUIRED )
set(Boost_INCLUDE_DIR ~/boost)
include_directories(/usr/include/mysql) 
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES src/httpServer.cpp)

add_executable (Server
    ${BOOST_BEAST_FILES}
     ${SOURCE_FILES}
    src/Jamfile
    src/fields_alloc.hpp
    src/httpServer.cpp
)

target_link_libraries(Server ${Boost_LIBRARIES} ${OpenCV_LIBS} -lpthread -lmysqlclient)
