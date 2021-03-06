cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(mesher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
if(MSVC)
	add_definitions(-DCONSTEXPR=const -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
else()
	add_definitions(-DCONSTEXPR=constexpr)
	set(CXX_WARNINGS "-Wall -Wextra -Wpointer-arith -Wcast-align -fstrict-aliasing -Wno-unused-local-typedefs -Wno-misleading-indentation -Wno-maybe-uninitialized -Wno-int-in-bool-context -Wno-implicit-fallthrough")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_WARNINGS} -fvisibility-inlines-hidden -O3")
endif()

include_directories(
	src/core
	src/utility
	src/thirdparty/glew-2.1.0/include
	src/thirdparty/glfw-3.2.1/include
	src/thirdparty/glm-0.9.8.5
	${OPENGL_INCLUDE_DIR}
)

add_library(core
	src/core/Mesher.cpp
)

add_executable(mesher
	src/mesher/mesher.cpp
	src/utility/Camera.cpp
	src/utility/FPS.cpp
	src/utility/Toggle.cpp
	src/utility/OGL.cpp
)
target_link_libraries(mesher
	core
	glew_s
	glfw
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/vertex.glsl ${CMAKE_CURRENT_BINARY_DIR}/shader/vertex.glsl COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/fragment.glsl ${CMAKE_CURRENT_BINARY_DIR}/shader/fragment.glsl COPYONLY)



option(BUILD_UTILS "utilities" OFF)
add_subdirectory(src/thirdparty/glew-2.1.0/build/cmake)

option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
add_subdirectory(src/thirdparty/glfw-3.2.1)
