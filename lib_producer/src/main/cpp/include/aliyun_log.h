//
// Created on 2023/12/22.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef aliyun_log_harmony_sdk_aliyun_log_H
#define aliyun_log_harmony_sdk_aliyun_log_H
#include "log_inner_include.h"
#include "stdint.h"
#include <stdint.h>
#include <unistd.h>
#include "aliyun-log-c-sdk/log_define.h"

LOG_CPP_START

typedef struct _aliyun_log {
    void* config;
    void* client;
    void* producer;
} aliyun_log;

typedef void(*aliyun_log_send_done_function)(
    const char* config_name, 
    uint32_t result, 
    size_t log_byte, 
    size_t compressed_bytes,
    const char* error_message,
    void* user_param
);

aliyun_log* aliyun_log_create(
    const char* endpoint,
    const char* project,
    const char* logstore,
    const char* access_key_id,
    const char* access_key_secret,
    const char* access_key_token
);

void aliyun_log_destroy(aliyun_log* aliyun_log);

void aliyun_log_set_send_done_function(aliyun_log_send_done_function send_done_function);

void aliyun_log_set_os_http_function(int (*f)(const char *url,
                                              char ***header_array,
                                              int header_count,
                                              const void *data,
                                              int data_len,
                                              post_log_result *http_response));

uint32_t aliyun_log_add_log(aliyun_log* aliyun_log, uint32_t length, char** key_values);

void aliyun_log_update_server_time(unsigned int time);

void aliyun_log_set_accesskey(
    aliyun_log *aliyun_log,
    const char* access_key_id, 
    const char* access_key_secret, 
    const char* access_key_token
);

void aliyun_log_set_topic(aliyun_log *aliyun_log, const char* topic);

void aliyun_log_set_source(aliyun_log *aliyun_log, const char* source);

void aliyun_log_add_tag(aliyun_log *aliyun_log, const char* key, const char* value);

void aliyun_log_debuggable(aliyun_log *aliyun_log, int32_t debuggable);

LOG_CPP_END

#endif //aliyun_log_harmony_sdk_aliyun_log_H
