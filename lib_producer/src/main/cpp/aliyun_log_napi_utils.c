//
// Created on 2024/2/1.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".
#include "aliyun_log_napi_utils.h"

int aliyun_log_napi_create_string_utf8(napi_env env, const char** str, size_t length, napi_value *result)
{
    if (napi_ok != napi_create_string_utf8(env, *str, length, result))
    {
        napi_throw_error(env, NULL, "Failed to napi_create_string_utf8");
        return 0;
    }
    return 1;
}

int aliyun_log_napi_create_int32(napi_env env, int32_t value, napi_value *result)
{
    if (napi_ok != napi_create_int32(env, value, result))
    {
        napi_throw_error(env, NULL, "Failed to napi_create_int32");
        return 0;
    }
    return 1;
}