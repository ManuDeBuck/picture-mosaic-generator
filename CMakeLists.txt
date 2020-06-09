cmake_minimum_required (VERSION 3.6)

project (collage)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c main.h stb_image/stb_image_resize.h stb_image/stb_image.h stb_image/stb_image_write.h)

add_executable (collage ${SOURCE_FILES})

target_link_libraries(collage m)