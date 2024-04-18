//
// Created on 2024/2/1.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef aliyun_log_harmony_sdk_aliyun_log_napi_utils_H
#define aliyun_log_harmony_sdk_aliyun_log_napi_utils_H
#include "napi/native_api.h"

//    work_data *data = (work_data *) work->data;
//    napi_value log_store;
//    napi_create_string_utf8(env, data->logstore_name, strlen(data->logstore_name), &log_store);
//    napi_value result_code;
//    napi_create_int32(env, data->result_code, &result_code);
//    napi_value log_byte;
//    napi_create_int32(env, data->log_byte, &log_byte);
//    napi_value compressed_bytes;
//    napi_create_int32(env, data->compressed_bytes, &compressed_bytes);
//    napi_value error_message;
//    napi_create_string_utf8(env, data->error_message, strlen(data->error_message), &error_message);

int aliyun_log_napi_create_string_utf8(napi_env env, const char** value, size_t length, napi_value *result);

int aliyun_log_napi_create_int32(napi_env env, int32_t value, napi_value *result);

#endif //aliyun_log_harmony_sdk_aliyun_log_napi_utils_H
