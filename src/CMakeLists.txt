find_package(Freetype)
if (FREETYPE_FOUND)
  include_directories(${FREETYPE_INCLUDE_DIRS})
  message(STATUS "${FREETYPE_LIBRARIES}")
endif (FREETYPE_FOUND)

find_package(LibXml2)
if (LIBXML2_FOUND)
  include_directories(${LIBXML2_INCLUDE_DIR})
endif (LIBXML2_FOUND)

find_package(ImageMagick COMPONENTS Magick++)
if (ImageMagick_FOUND)
  include_directories(${ImageMagick_INCLUDE_DIRS})
endif (ImageMagick_FOUND)

add_library(text text.cpp)
target_link_libraries(text OpenVG ${FREETYPE_LIBRARIES})

add_library(window window.cpp)
target_link_libraries(window EGL OpenVG GLESv2)
if (PLATFORM STREQUAL "PI")
  target_link_libraries(window bcm_host)
endif ()

add_library(weather weather.cpp)
target_link_libraries(weather xml2 OpenVG Magick++)

add_executable(rpresent rpresent.cpp)
target_link_libraries(rpresent text window)

add_executable(weather_main weather_main.cpp)
target_link_libraries(weather_main text weather window)
