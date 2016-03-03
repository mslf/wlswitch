cmake_minimum_required(VERSION 3.3)	

project(wlswitch)			
# Project name

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wcpp")
# For regex needs C++11

find_package(ImageMagick COMPONENTS Magick++)
if(NOT ImageMagick_Magick++_FOUND)
	message(SEND_ERROR "Failed to find Magick++")
	return()
else()
	include_directories(${ImageMagick_INCLUDE_DIRS})
endif()
# wlswitch needs Magick++ libs and includes

include_directories(./include)
#Adding include directories

set(SOURCE_EXUTABLE ./src/main.cpp)		

set(SOURCE_Wlswitch_LIB ./src/wlswitch.cpp)
set(SOURCE_Marker_LIB ./src/marker.cpp)

add_library(wlswitch_lib STATIC ${SOURCE_Wlswitch_LIB})
add_library(marker_lib STATIC ${SOURCE_Marker_LIB})

add_executable(wlswitch ${SOURCE_EXUTABLE})	

target_link_libraries(wlswitch_lib ${ImageMagick_LIBRARIES})
target_link_libraries(wlswitch_lib marker_lib)
target_link_libraries(wlswitch wlswitch_lib)