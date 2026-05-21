cmake_minimum_required(VERSION 3.10)

project(NeuralPixel C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
find_package(PNG REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

aux_source_directory(src SOURCES)
add_executable(neural_pixel ${SOURCES})

target_include_directories(neural_pixel PRIVATE ${GTK4_INCLUDE_DIRS})

target_link_libraries(neural_pixel PRIVATE ${GTK4_LIBRARIES})

target_compile_options(neural_pixel PRIVATE ${GTK4_CFLAGS})

target_link_libraries(neural_pixel PRIVATE PNG::PNG)

if (WIN32)
	target_link_libraries(neural_pixel PRIVATE ws2_32)
endif()

file(COPY resources/ DESTINATION ${CMAKE_BINARY_DIR}/resources)

