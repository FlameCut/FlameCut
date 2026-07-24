cmake_minimum_required( VERSION 3.22 )

project(flmaecut LANGUAGES C CXX)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY bin/ )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY obj/ )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY archive/ )

enable_testing()

find_package(Vulkan REQUIRED)

add_subdirectory("${CMAKE_SOURCE_DIR}/lib/SDL3-3.4.12" EXCLUDE_FROM_ALL)

file(GLOB_RECURSE FLAME_SOURCE 
    "${CMAKE_SOURCE_DIR}/src/*.h" 
    "${CMAKE_SOURCE_DIR}/src/*.hpp" 
    "${CMAKE_SOURCE_DIR}/src/*.c" 
    "${CMAKE_SOURCE_DIR}/src/*.cpp")

file(GLOB_RECURSE IMGUI_SOURCE 
    "${CMAKE_SOURCE_DIR}/lib/imgui-1.92.8/*.c" 
    "${CMAKE_SOURCE_DIR}/lib/imgui-1.92.8/*.cpp")

add_executable(flamecut_app
    ${FLAME_SOURCE}
    ${IMGUI_SOURCE}
)

target_include_directories(flamecut_app PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/lib/SDL3-3.4.12/include"
    "${CMAKE_SOURCE_DIR}/lib/imgui-1.92.8"
    "${CMAKE_SOURCE_DIR}/lib/imgui-1.92.8/misc/cpp"
    "${CMAKE_SOURCE_DIR}/lib/imgui-1.92.8/misc/freetype"
    "${CMAKE_SOURCE_DIR}/lib/imgui-1.92.8/backends"
)

target_link_libraries(flamecut_app 
    SDL3
    Vulkan::Vulkan
)