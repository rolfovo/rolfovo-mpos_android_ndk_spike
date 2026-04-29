cmake_minimum_required(VERSION 3.22.1)

project(mpos_android_spike LANGUAGES C CXX)

get_filename_component(WORKSPACE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../../../" ABSOLUTE)
set(LVGL_DIR "${WORKSPACE_DIR}/_tmp_mpos/lvgl_micropython/lib/lvgl")

if(NOT EXISTS "${LVGL_DIR}/lvgl.h")
    message(FATAL_ERROR "LVGL checkout not found at ${LVGL_DIR}. Run git submodule update in _tmp_mpos first.")
endif()

file(GLOB_RECURSE LVGL_SOURCES CONFIGURE_DEPENDS
    "${LVGL_DIR}/src/*.c"
)

add_library(mpos_android_spike SHARED
    native-lib.cpp
    ${LVGL_SOURCES}
)

target_include_directories(mpos_android_spike PRIVATE
    "${LVGL_DIR}"
    "${LVGL_DIR}/src"
)

target_compile_definitions(mpos_android_spike PRIVATE
    LV_CONF_SKIP=1
    LV_COLOR_DEPTH=32
    LV_MEM_SIZE=4194304U
    LV_DEF_REFR_PERIOD=16
    LV_USE_LOG=1
    LV_LOG_LEVEL=LV_LOG_LEVEL_WARN
)

target_compile_options(mpos_android_spike PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
)

target_link_libraries(mpos_android_spike
    android
    log
)
