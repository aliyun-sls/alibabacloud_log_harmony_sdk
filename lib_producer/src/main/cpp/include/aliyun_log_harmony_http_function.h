//
// Created on 2024/1/29.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef aliyun_log_harmony_sdk_aliyun_log_harmony_http_H
#define aliyun_log_harmony_sdk_aliyun_log_harmony_http_H
#include "log_define.h"

int napi_os_http_function(const char *url, char ***header_array, int header_count,
            const void *data, int data_len, post_log_result *http_response);

#endif //aliyun_log_harmony_sdk_aliyun_log_harmony_http_H