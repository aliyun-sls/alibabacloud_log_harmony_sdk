//
// Created on 2024/2/4.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef aliyun_log_harmony_sdk_aliyun_log_native_logger_H
#define aliyun_log_harmony_sdk_aliyun_log_native_logger_H
#include "inner_log.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0x3363  // 全局domain宏，标识业务领域
#define LOG_TAG "aliyun_log_native"   // 全局tag宏，标识模块日志tag

#endif //aliyun_log_harmony_sdk_aliyun_log_native_logger_H
