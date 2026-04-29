cmake_minimum_required(VERSION 3.22.1)

project(mpos_android_spike LANGUAGES C CXX)

get_filename_component(WORKSPACE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../../../" ABSOLUTE)
set(LVGL_DIR "${WORKSPACE_DIR}/_tmp_mpos/lvgl_micropython/lib/lvgl")
set(MICROPY_DIR "${WORKSPACE_DIR}/_tmp_mpos/lvgl_micropython/lib/micropython")
set(MICROPY_PORT_DIR "${MICROPY_DIR}/ports/embed")
set(MICROPY_TARGET mpos_android_spike)
set(MICROPY_QSTRDEFS_PORT "${CMAKE_CURRENT_LIST_DIR}/micropython_port/qstrdefsport.h")

if(NOT EXISTS "${LVGL_DIR}/lvgl.h")
    message(FATAL_ERROR "LVGL checkout not found at ${LVGL_DIR}. Run git submodule update in _tmp_mpos first.")
endif()

if(NOT EXISTS "${MICROPY_DIR}/py/py.cmake")
    message(FATAL_ERROR "MicroPython checkout not found at ${MICROPY_DIR}. Run git submodule update in _tmp_mpos first.")
endif()

include("${MICROPY_DIR}/py/py.cmake")

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    if(CMAKE_C_COMPILER_TARGET)
        set(MICROPY_ANDROID_CLANG_TARGET "${CMAKE_C_COMPILER_TARGET}")
    elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
        set(MICROPY_ANDROID_CLANG_TARGET "aarch64-none-linux-android${CMAKE_SYSTEM_VERSION}")
    elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
        set(MICROPY_ANDROID_CLANG_TARGET "armv7-none-linux-androideabi${CMAKE_SYSTEM_VERSION}")
    elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86")
        set(MICROPY_ANDROID_CLANG_TARGET "i686-none-linux-android${CMAKE_SYSTEM_VERSION}")
    elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
        set(MICROPY_ANDROID_CLANG_TARGET "x86_64-none-linux-android${CMAKE_SYSTEM_VERSION}")
    endif()

    if(MICROPY_ANDROID_CLANG_TARGET)
        list(APPEND MICROPY_CPP_FLAGS_EXTRA "--target=${MICROPY_ANDROID_CLANG_TARGET}")
    endif()

    if(CMAKE_SYSROOT)
        list(APPEND MICROPY_CPP_FLAGS_EXTRA "--sysroot=${CMAKE_SYSROOT}")
    endif()

    list(APPEND MICROPY_CPP_FLAGS_EXTRA -DANDROID)
endif()

file(GLOB_RECURSE LVGL_SOURCES CONFIGURE_DEPENDS
    "${LVGL_DIR}/src/*.c"
)

set(MICROPY_SOURCE_PORT
    "${MICROPY_DIR}/ports/embed/port/embed_util.c"
    "${MICROPY_DIR}/ports/embed/port/mphalport.c"
    "${MICROPY_DIR}/shared/runtime/gchelper_generic.c"
    "${CMAKE_CURRENT_LIST_DIR}/micropython_port/modmpos.c"
)

set(MICROPY_SOURCE_QSTR
    ${MICROPY_SOURCE_PY}
    ${MICROPY_SOURCE_PORT}
)

add_library(mpos_android_spike SHARED
    native-lib.cpp
    ${LVGL_SOURCES}
    ${MICROPY_SOURCE_PY}
    ${MICROPY_SOURCE_PORT}
)

target_include_directories(mpos_android_spike PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}"
    "${LVGL_DIR}"
    "${LVGL_DIR}/src"
    "${MICROPY_DIR}"
    "${MICROPY_PORT_DIR}"
    "${MICROPY_PORT_DIR}/port"
    "${MICROPY_DIR}/shared/runtime"
    "${CMAKE_BINARY_DIR}"
    "${CMAKE_BINARY_DIR}/genhdr"
    "${CMAKE_CURRENT_LIST_DIR}/micropython_port"
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
    m
)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/micropython_mkrules_android.cmake")
