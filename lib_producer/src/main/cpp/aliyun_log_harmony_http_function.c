//
// Created on 2024/1/29.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "aliyun_log_harmony_http_function.h"
#include "aliyun_log.h"
#include "inner_log.h"
#include "log_sds.h"
#include <stdio.h>
#include <curl/curl.h>
#include <string.h>

static void processUnixTimeFromHeader(char * ptr, int size)
{
    char buf[64];
    memset(buf, 0, sizeof(buf));
    int i = 0;
    for (i = 0; i < size; ++i)
    {
        if (ptr[i] < '0' || ptr[i] > '9')
        {
            continue;
        }
        else
        {
            break;
        }
    }
    int j = 0;
    for (j = 0; j < 60 && i < size; ++j, ++i)
    {
        if (ptr[i] >= '0' && ptr[i] <= '9')
        {
            buf[j] = ptr[i];
        }
        else
        {
            break;
        }
    }
    long long serverTime = atoll(buf);
    if (serverTime <= 1500000000 || serverTime > 4294967294)
    {
        // invalid time
        return;
    }
    int deltaTime = serverTime - (long long)time(NULL);
    if (deltaTime > 30 || deltaTime < -30)
    {
//        log_set_local_server_real_time(serverTime);
        aliyun_log_update_server_time(serverTime);
    }
}

size_t header_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t totalLen = size * nmemb;
    log_sds * buffer = (log_sds *)stream;


    // only copy header start with x-log-
    if (totalLen > 6 && memcmp(ptr, "x-log-", 6) == 0)
    {
        *buffer = log_sdscpylen(*buffer, (const char*)ptr, totalLen);
    }
    if (totalLen > 10 && memcmp(ptr, "x-log-time", 10) == 0)
    {
        processUnixTimeFromHeader((char *)ptr + 10, totalLen - 10);
    }
    return totalLen;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t totalLen = size * nmemb;
    //printf("body  ---->  %d  %s \n", (int) (totalLen), (const char*) ptr);
    log_sds * buffer = (log_sds *)stream;
    if (*buffer == NULL)
    {
        *buffer = log_sdsnewEmpty(256);
    }
    *buffer = log_sdscpylen(*buffer, (const char*)ptr, totalLen);
    return totalLen;
}


int debug_callback(CURL *handle,
                   curl_infotype type,
                   char *data,
                   size_t size,
                   void *userptr)
{
    switch (type)
    {
        case CURLINFO_TEXT:
        {
            aos_trace_log("CURLINFO_TEXT: %s", data);
            break;    
        }
        case CURLINFO_HEADER_IN:
        {
            aos_trace_log("CURLINFO_HEADER_IN: %s", data);
            break;    
        }
        case CURLINFO_HEADER_OUT:
        {
            aos_trace_log("CURLINFO_HEADER_OUT: %{private}s", data);
            break;    
        }
        case CURLINFO_DATA_IN:
        case CURLINFO_SSL_DATA_IN:
        {
            aos_trace_log("CURLINFO_DATA_IN: size: %d, %s", size, data);
            break;    
        }
        case CURLINFO_DATA_OUT:
        case CURLINFO_SSL_DATA_OUT:
        {
            aos_trace_log("CURLINFO_DATA_OUT: size: %d, %{private}s", size, data);
            break;    
        }
        default:
            return 0;
    }
    return 0;
}

int napi_os_http_function(const char *url, char ***header_array, int header_count,
            const void *data, int data_len, post_log_result *http_response)
{
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        return -1;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    
    size_t request_id_len = sizeof(char) * 256;
    http_response->requestID = (char*) malloc(request_id_len);
    memset(http_response->requestID, 0, request_id_len);
    
    size_t error_message_len = sizeof(char) * 256;
    http_response->errorMessage = (char*) malloc(error_message_len);
    memset(http_response->errorMessage, 0, error_message_len);
    
    // headers
    struct curl_slist *headers = NULL;
    char ** original_header_array = *header_array;
    for(int i = 0; i < header_count; i++) 
    {
        char *kv = original_header_array[i];
        if (NULL != kv)
        {
            char *eq = strchr(kv, ':');
            if (NULL != eq && eq != kv && eq[1] != 0)
            {
                *eq = 0;
                size_t key_len = strlen(kv) + 1;
                size_t value_ken = strlen(eq+1) + 1;
                char header[key_len + value_ken];
                snprintf(header, sizeof(header), "%s:%s", kv, eq+1);
                headers = curl_slist_append(headers, header);
                *eq = '=';
            }
        }
    }
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, NULL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    // post method
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    
    // post body
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)data);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data_len);
    
    curl_easy_setopt(curl, CURLOPT_FILETIME, 1);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1);
    curl_easy_setopt(curl, CURLOPT_NETRC, CURL_NETRC_IGNORED);
    
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "aliyun-log-openharmony/0.3.0");
    
    // timeout: 20
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);
    
    log_sds header = log_sdsnewEmpty(64);
    char* body = NULL;
    
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    
    // enable debug info 
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_callback);

    CURLcode res = curl_easy_perform(curl);
    long http_code;
    if (CURLE_OK == res)
    {
        if (CURLE_OK != (res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code)))
        {
            aos_warn_log("curl: get info error. result: %s", curl_easy_strerror(res));
            http_response->statusCode = -1;
        }
        else
        {
            http_response->statusCode = http_code;
        }
    }
    else
    {
        if (NULL == body)
        {
            body = log_sdsnew(curl_easy_strerror(res));
        }
        else
        {
            body = log_sdscpy(body, curl_easy_strerror(res));
        }
        http_response->statusCode = -1 * (int)res;
        strncpy(http_response->errorMessage, body, error_message_len);
    }
    
    if (log_sdslen(header) > 0)
    {
        strncpy(http_response->requestID, header, request_id_len);
    }
    else
    {
        log_sdsfree(header);
        header = NULL;
        strcpy(http_response->requestID, "");
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return http_response->statusCode;
}
