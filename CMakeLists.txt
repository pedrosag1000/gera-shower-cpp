cmake_minimum_required(VERSION 3.1)

project( DisplayImage )
set(THREADS_PREFER_PTHREADS_FLAG ON)
#SET(CMAKE_CXX_FLAGS "-fsanitize=address")

find_package( OpenCV REQUIRED )
find_package(CppLinuxSerial REQUIRED)
find_package(Threads REQUIRED)

include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( DisplayImage DisplayImage.cpp )
target_link_libraries( DisplayImage ${OpenCV_LIBS} )
target_link_libraries( DisplayImage CppLinuxSerial::CppLinuxSerial)
target_link_libraries( DisplayImage Threads::Threads)

configure_file(images/software-splash.png splash.png COPYONLY)
