cmake_minimum_required(VERSION 3.16)
project(ProjetImageHarmony)

find_package(OpenGL REQUIRED)
find_package(OpenMP REQUIRED)
add_subdirectory(external)

include_directories(
    external
    external/imgui
    external/glfw-3.1.2/include
    external/glew-1.13.0/include
    external/maxflow-v3.04.src
    src
    common
)


set(IMGUI_SRC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_impl_glfw.cpp
    external/imgui/imgui_impl_opengl3.cpp
)

set(ALL_LIBS
    ${OPENGL_LIBRARY}
    glfw
    GLEW_1130
)

add_executable(ProjetImage
    main.cpp
    src/image.cpp
    src/template.cpp
    src/interface.cpp
    src/rendering.cpp
    src/harmonization.cpp
    common/stb_image.cpp
    external/maxflow-v3.04.src/graph.cpp
    external/maxflow-v3.04.src/maxflow.cpp
    ${IMGUI_SRC}
)

target_link_libraries(ProjetImage
    OpenMP::OpenMP_CXX
    ${ALL_LIBS}
)

add_definitions(
    -DGLEW_STATIC
    -D_CRT_SECURE_NO_WARNINGS
)