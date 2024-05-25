cmake_minimum_required (VERSION 3.8)

project(panotool C)

add_executable(panotool "main.c")

cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)
target_link_libraries(panotool OpenGL::GL)

find_package(GLEW REQUIRED)
target_link_libraries(panotool GLEW::glew)

find_package(glfw3 REQUIRED)
target_link_libraries(panotool glfw)

