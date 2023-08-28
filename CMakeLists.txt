cmake_minimum_required(VERSION 3.1)

project( DisplayImage )
set(THREADS_PREFER_PTHREADS_FLAG ON)
find_package( OpenCV REQUIRED )

set(BUILD_TESTS FALSE)

include(FetchContent)

FetchContent_Declare(
        task-thread-pool
        GIT_REPOSITORY https://github.com/alugowski/task-thread-pool
        GIT_TAG main
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(task-thread-pool)


FetchContent_Declare(
        CppLinuxSerial
        GIT_REPOSITORY https://github.com/gbmhunter/CppLinuxSerial
        GIT_TAG master
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(CppLinuxSerial)


find_package(Threads REQUIRED)

include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( DisplayImage DisplayImage.cpp )
target_link_libraries( DisplayImage ${OpenCV_LIBS} )
target_link_libraries( DisplayImage CppLinuxSerial::CppLinuxSerial)
target_link_libraries( DisplayImage Threads::Threads)
