cmake_minimum_required(VERSION 2.8)

if (PLATFORM STREQUAL "RaspberryPi")
  message(STATUS "Configuring for RaspberryPi build")
  set(CMAKE_C_COMPILER "arm-linux-gnueabi-gcc")
  set(CMAKE_CXX_COMPILER "arm-linux-gnueabi-g++")
elseif (PLATFORM STREQUAL "X11")
  message(STATUS "Configuring for X11 build")
endif ()

project(rpresent)

if (PLATFORM STREQUAL "RaspberryPi")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_PI")
elseif (PLATFORM STREQUAL "X11")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_X11")
endif ()

add_subdirectory(src)