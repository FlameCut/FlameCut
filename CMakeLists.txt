cmake_minimum_required( VERSION 3.22 )

project(flmaecut LANGUAGES C CXX)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY bin/ )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY obj/ )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY archive/ )

enable_testing()

find_package(Vulkan REQUIRED)

add_subdirectory("${CMAKE_SOURCE_DIR}/lib/SDL3-3.4.12" EXCLUDE_FROM_ALL)
add_subdirectory("${CMAKE_SOURCE_DIR}/lib/spdlog-1.17.0" EXCLUDE_FROM_ALL)
add_subdirectory("${CMAKE_SOURCE_DIR}/lib/stduuid-1.2.3" EXCLUDE_FROM_ALL)

set(FFMPEG_LOCAL_DIR "${CMAKE_SOURCE_DIR}/lib/ffmpeg-9.0/dist")
find_library(AVCODEC_LIB NAMES avcodec PATHS "${FFMPEG_LOCAL_DIR}/lib" NO_DEFAULT_PATH REQUIRED)
find_library(AVDEVICE_LIB NAMES avdevice PATHS "${FFMPEG_LOCAL_DIR}/lib" NO_DEFAULT_PATH REQUIRED)
find_library(AVFILTER_LIB NAMES avfilter PATHS "${FFMPEG_LOCAL_DIR}/lib" NO_DEFAULT_PATH REQUIRED)
find_library(AVFORMAT_LIB NAMES avformat PATHS "${FFMPEG_LOCAL_DIR}/lib" NO_DEFAULT_PATH REQUIRED)
find_library(AVUTIL_LIB NAMES avutil PATHS "${FFMPEG_LOCAL_DIR}/lib" NO_DEFAULT_PATH REQUIRED)
find_library(SWRESAMPLE_LIB NAMES swresample PATHS "${FFMPEG_LOCAL_DIR}/lib" NO_DEFAULT_PATH REQUIRED)
find_library(SWSCALE_LIB NAMES swscale PATHS "${FFMPEG_LOCAL_DIR}/lib" NO_DEFAULT_PATH REQUIRED)

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

target_include_directories(flamecut_app PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/lib/SDL3-3.4.12/include"
    "${CMAKE_SOURCE_DIR}/lib/imgui-1.92.8"
    "${CMAKE_SOURCE_DIR}/lib/imgui-1.92.8/misc/cpp"
    "${CMAKE_SOURCE_DIR}/lib/imgui-1.92.8/misc/freetype"
    "${CMAKE_SOURCE_DIR}/lib/imgui-1.92.8/backends"
    "${CMAKE_SOURCE_DIR}/stduuid-1.2.3/include"
    "${CMAKE_SOURCE_DIR}/spdlog-1.17.0/include"
    "${FFMPEG_LOCAL_DIR}/include"
)

target_link_libraries(flamecut_app 
    SDL3
    Vulkan::Vulkan
    stduuid
    spdlog
    ${AVCODEC_LIB}
    ${AVDEVICE_LIB}
    ${AVFILTER_LIB}
    ${AVFORMAT_LIB}
    ${AVUTIL_LIB}
    ${SWRESAMPLE_LIB}
    ${SWSCALE_LIB}
)