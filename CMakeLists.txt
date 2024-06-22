cmake_minimum_required(VERSION 3.8)
project(soundvis)

find_package(SDL2 REQUIRED)

set(SRC ${CMAKE_SOURCE_DIR}/src)


add_executable(soundvis
	${SRC}/main.cpp
)
target_include_directories(
	soundvis PUBLIC
	${SDL2_INCLUDE_DIRS}
)
target_link_libraries(
	soundvis PUBLIC
	${SDL2_LIBRARIES}
)
