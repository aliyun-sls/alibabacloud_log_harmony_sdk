
cmake_minimum_required(VERSION 3.4.1)

project(logcsdk)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${NATIVERENDER_ROOT_PATH}
                    ${NATIVERENDER_ROOT_PATH}/include)

add_library(logcsdk STATIC
            inner_log.c
            log_api.c
            log_builder.c
            log_hmac-sha.c
            log_http_interface.c
            log_lz4.c
            log_md5.c
            log_persistent_manager.c
            log_producer_client.c
            log_producer_common.c
            log_producer_config.c
            log_producer_manager.c
            log_producer_sender.c
            log_queue.c
            log_ring_file.c
            log_sds.c
            log_sha1.c
            log_util.c
            log_zstd.c
)

target_link_libraries(logcsdk libhilog_ndk.z.so)



#aux_source_directory(. DIR_LIB_SRCS)

#message(STATUS ${DIR_LIB_SRCS})

#add_library(logcsdk STATIC ${DIR_LIB_SRCS})