# the minimum version of CMake.
cmake_minimum_required(VERSION 3.4.1)
project(aliyunlogsdk)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-DHarmonyOS)

add_subdirectory(aliyun-log-c-sdk)
include_directories(
    ${NATIVERENDER_ROOT_PATH}
    ${NATIVERENDER_ROOT_PATH}/include
    ${NATIVERENDER_ROOT_PATH}/aliyun-log-c-sdk
    ${NATIVERENDER_ROOT_PATH}/thirdparty/curl/include
)

add_library(aliyunlog SHARED
    aliyun_log_native_logger.c
    aliyun_log.c
    aliyun_log_napi_utils.c
    aliyun_log_harmony_http_function.c
    aliyun_log_napi_callback.c
    aliyun_log_napi.cpp
)

target_link_libraries(aliyunlog PUBLIC
    libace_napi.z.so
    libhilog_ndk.z.so
    uv
    logcsdk
    ${NATIVERENDER_ROOT_PATH}/../../../libs/${OHOS_ARCH}/libcrypto.a
    ${NATIVERENDER_ROOT_PATH}/../../../libs/${OHOS_ARCH}/libssl.a
    ${NATIVERENDER_ROOT_PATH}/../../../libs/${OHOS_ARCH}/libcurl.a
    z
)
