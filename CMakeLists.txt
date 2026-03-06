cmake_minimum_required(VERSION 3.16)

project(ProjetImageHarmony VERSION 1.0 LANGUAGES CXX)

add_executable(ProjetImage
    main.cpp
    src/image.cpp
    src/template.cpp
    common/stb_image.cpp
)

target_compile_features(ProjetImage PRIVATE cxx_std_17)

target_include_directories(ProjetImage PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/common
)