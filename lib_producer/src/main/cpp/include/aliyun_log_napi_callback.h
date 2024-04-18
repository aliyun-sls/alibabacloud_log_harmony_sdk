//
// Created on 2024/2/1.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef aliyun_log_harmony_sdk_aliyun_log_napi_callback_H
#define aliyun_log_harmony_sdk_aliyun_log_napi_callback_H

#include "napi/native_api.h"

typedef struct _napi_callback_reference {
    napi_env env;
    napi_ref callback_ref;
    napi_ref this_ref;
} napi_callback_reference;

void aliyun_log_napi_send_done_function(
    const char *config_name,
    uint32_t result,
    size_t log_byte,
    size_t compressed_bytes,
    const char *error_message,
    void *user_param);

napi_value aliyun_log_napi_set_callback(napi_env env, napi_callback_info info);

#endif // aliyun_log_harmony_sdk_aliyun_log_napi_callback_H
