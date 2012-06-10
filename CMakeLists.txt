cmake_minimum_required(VERSION 2.8)

if (PLATFORM STREQUAL "PI")
  message(STATUS "Configuring for RaspberryPi build")
elseif (PLATFORM STREQUAL "X11")
  message(STATUS "Configuring for X11 build")
endif ()

project(rpresent)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
if (PLATFORM STREQUAL "PI")
  include_directories(/opt/vc/include)
  include_directories(/opt/vc/include/interface/vcos/pthreads)
  link_directories(/opt/vc/lib)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_PI")
elseif (PLATFORM STREQUAL "X11")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_X11")
endif ()

add_subdirectory(src)