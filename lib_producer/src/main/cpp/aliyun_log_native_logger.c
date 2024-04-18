//
// Created on 2024/2/4.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "aliyun_log_native_logger.h"
#include <hilog/log.h>

void aos_print_log_harmony(int level, char*log)
{
    if (AOS_LOG_TRACE == level || AOS_LOG_DEBUG == level)
    {
        OH_LOG_DEBUG(LOG_APP, "%{public}s", log);
    }
    else if (AOS_LOG_INFO == level)
    {
        OH_LOG_INFO(LOG_APP, "%{public}s", log);
    }
    else if (AOS_LOG_WARN == level)
    {
        OH_LOG_WARN(LOG_APP, "%{public}s", log);
    }
    else if (AOS_LOG_ERROR == level)
    {
        OH_LOG_ERROR(LOG_APP, "%{public}s", log);
    }
    else if (AOS_LOG_FATAL == level)
    {
        OH_LOG_FATAL(LOG_APP, "%{public}s", log);
    }
}