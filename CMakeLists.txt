# CMake powered build system for data structure.
# Create by Jaemyun Kim
# First draft is Oct. 15, 2017

CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT (KakaoTalk)


# set output path both execution and library files.
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# prevent warnings on visual studio.
if(WIN32)
	if(MSVC)
		# for _CRT_SECURE_NO_WARNINGS
		add_definitions(-D_CRT_SECURE_NO_WARNINGS)
		
		# for _SCL_SECURE_NO_WARNINGS warning
		add_definitions(-D_SCL_SECURE_NO_WARNINGS)
	endif(MSVC)
endif(WIN32)


# use c++11 version
set(CMAKE_CXX_STANDARD 11) # C++11...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11


# include sub-directories. Target directories have to have "CMakeLists.txt" file.
ADD_SUBDIRECTORY(src)


SET(CMAKE_VERBOSE_MAKEFILE ON)
