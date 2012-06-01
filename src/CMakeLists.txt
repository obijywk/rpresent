include(FindFreetype)
include_directories(${FREETYPE_INCLUDE_DIRS})

add_library(text text.cpp)
target_link_libraries(text OpenVG ${FREETYPE_LIBRARIES})

add_library(window window.cpp)
target_link_libraries(window EGL OpenVG GLESv2)
if (PLATFORM STREQUAL "PI")
  target_link_libraries(window bcm_host)
endif ()

add_executable(rpresent rpresent.cpp)
target_link_libraries(rpresent text window)