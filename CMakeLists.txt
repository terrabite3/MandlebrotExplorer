# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (MandelbrotGame)

find_package(OpenGL REQUIRED)

find_package(OpenCL REQUIRED)


if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()



# Compile external dependencies 
add_subdirectory (external)

# On Visual 2005 and above, this module can set the debug working directory
cmake_policy(SET CMP0026 OLD)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-fe2273")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile) # /MP




include_directories(
	external/glfw-3.1.2/include/GLFW/
	external/glm-0.9.7.1/
	external/glew-1.13.0/include/
	.
	${OpenCL_INCLUDE_DIRS}
)


add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

SET(HEADER
	common/shader.hpp
	common/texture.hpp

	src/Camera.h
	src/OpenClRenderer.h
	src/CpuRenderer.h
	src/Screen.h
	src/Tile.h
)

set(SOURCE
	common/shader.cpp
	common/texture.cpp

	src/Camera.cpp
	src/OpenClRenderer.cpp
	src/CpuRenderer.cpp
	src/tutorial05.cpp
	src/Screen.cpp
	src/Tile.cpp
)

# Tutorial 5
add_executable(${PROJECT_NAME}
	${HEADER}
	${SOURCE}
)
target_link_libraries(${PROJECT_NAME}
	${OPENGL_LIBRARY}
	${OpenCL_LIBRARIES}
	glfw
	GLEW_1130
)







SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )

