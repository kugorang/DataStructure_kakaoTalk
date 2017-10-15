# CMake powered build system for data structure.
# Create by Jaemyun Kim
# First draft is Oct. 15, 2017

file (GLOB DS_PROJECT1_HDR "*.h*")
file (GLOB DS_PROJECT1_SRC "*.c*")


# Variables setting for this project ####################################
# set variable for this project
SET(PROJ_NAME KakaoTalk)
SET(PROJ_FILES ${DS_PROJECT1_HDR} ${DS_PROJECT1_SRC})


# set build target #############################################
SET_SOURCE_FILES_PROPERTIES(${PROJ_FILES}
	PROPERTIES
	COMPILE_FLAGS "-D__NO_UI__ -D_CRT_SECURE_NO_WARNINGS")
ADD_EXECUTABLE(${PROJ_NAME} ${PROJ_FILES})
TARGET_LINK_LIBRARIES(${PROJ_NAME} ${PROJ_LIBS})
