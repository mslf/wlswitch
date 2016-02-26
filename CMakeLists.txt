cmake_minimum_required(VERSION 3.3)	

project(wlswitch)			
# Project name

add_definitions(-std=c++11)
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

set(SOURCE_LIB ./src/wlswitch.cpp)			

add_library(wlswitch_lib STATIC ${SOURCE_LIB})	

add_executable(wlswitch ${SOURCE_EXUTABLE})	

target_link_libraries(wlswitch_lib ${ImageMagick_LIBRARIES})		
target_link_libraries(wlswitch wlswitch_lib)