cmake_minimum_required (VERSION 3.8)
set (CMAKE_CXX_STANDARD 23)

project(opengl)

add_executable(openglref "main.cpp" "Syslog.cpp" "Syslog.h" "Window.cpp" "Window.h" "Shader.cpp" "Shader.h" "Image.h" "Image.cpp" "Texture.h" "Texture.cpp" "Framebuffer.h" "Framebuffer.cpp" "Font.h" "Font.cpp" "UI.cpp" "UI.h" "3dMath/3dMath.h" "3dMath/3dMath.cpp" "3dMath/mat4.h" "3dMath/vec4.h" "3dMath/vec3.h" "3dMath/vec2.h" "3dMath/quat.h")

cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)
target_link_libraries(openglref OpenGL::GL)

find_package(GLEW REQUIRED)
target_link_libraries(openglref GLEW::glew)

find_package(glfw3 REQUIRED)
target_link_libraries(openglref glfw)

set(CMAKE_CXX_FLAGS "-O0 -ggdb")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O0 -ggdb")

