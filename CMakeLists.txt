cmake_minimum_required(VERSION 3.17)
project(tetris C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-O0")

add_executable(tetris main.c tetris.c tetris.h tetris.c tetris.h)
target_link_libraries(tetris PRIVATE ncursesw)
