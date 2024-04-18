
#include "napi/native_api.h"
#include "aliyun_log.h"
#include <cstdlib>
#include <time.h>
#include "uv.h"
extern "C" {
#include "aliyun_log_napi_utils.h"
#include "aliyun_log_harmony_http_function.h"
#include "aliyun_log_napi_callback.h"
#include "curl/curl.h"
}

static void _napi_get_value_string(napi_env env, napi_value arg, char** dest)
{
    size_t size = 0;
    napi_get_value_string_utf8(env, arg, nullptr, 0, &size);
    if (size <= 0) {
        *dest = nullptr;
        return;
    }
    
    *dest = (char*)malloc(sizeof(char) * (size + 1));
    size_t dest_size = 0;
    napi_get_value_string_utf8(env, arg, *dest, (size + 1), &dest_size);
}

static napi_value create_instance(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 6;
    size_t argc = 6;
    napi_value args[6] = {nullptr};
    
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (argc < requireArgc)
    {
        napi_throw_error(env, nullptr, "Wrong arguments. 6 arguments are expected.");    
        return nullptr;
    }
    
    char* endpoint;
    char* project;
    char* logstore;
    char* access_key_id;
    char* access_key_secret;
    char* access_key_token;
    
    _napi_get_value_string(env, args[0], &endpoint);
    _napi_get_value_string(env, args[1], &project);
    _napi_get_value_string(env, args[2], &logstore);
    _napi_get_value_string(env, args[3], &access_key_id);
    _napi_get_value_string(env, args[4], &access_key_secret);
    _napi_get_value_string(env, args[5], &access_key_token);
    
    CURLcode ecode;
    if ((ecode = curl_global_init(CURL_GLOBAL_ALL)) != CURLE_OK)
    {
//        aos_error_log("curl_global_init failure, code: %d %s.\n", ecode, curl_easy_strerror(ecode));
        return NULL;
    }
    
    aliyun_log *aliyun = aliyun_log_create(
        endpoint,
        project,
        logstore,
        access_key_id,
        access_key_secret,
        access_key_token
    );
    
    if (endpoint)
    {
        free(endpoint);
    }
    if (project)
    {
        free(project);
    }
    if (logstore)
    {
        free(logstore);
    }
    if (access_key_id)
    {
        free(access_key_id);
    }
    if (access_key_secret)
    {
        free(access_key_secret);
    }
    if (access_key_token)
    {
        free(access_key_token);
    }
    
    napi_value token = nullptr;
    napi_status status = napi_create_external(env, aliyun, NULL, NULL, &token);
    if (napi_ok != status)
    {
        napi_throw_error(env, NULL, "Failed to create external");
    }
    
    return token;
}

static napi_value destroy_instance(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 1;
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (argc < requireArgc)
    {
        napi_throw_error(env, nullptr, "Wrong arguments. 1 arguments are expected.");    
        return nullptr;
    }
    
    void *token;
    napi_status status = napi_get_value_external(env, args[0], &token);
    if (napi_ok != status)
    {
        napi_throw_error(env, nullptr, "Failed to get aliyun_log pointer");
        return NULL;
    }
    
    aliyun_log *aliyun = (aliyun_log*)token;
    aliyun_log_destroy(aliyun);

    curl_global_cleanup();

    return NULL;
    
}

static napi_value add_log(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 3;
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (argc < requireArgc)
    {
        napi_throw_error(env, nullptr, "Wrong arguments. 3 arguments are expected.");    
        return nullptr;
    }
    
    // get the aliyun_log* pointer
    void *token;
    napi_status status = napi_get_value_external(env, args[0], &token);
    if (napi_ok != status)
    {
        napi_throw_error(env, nullptr, "Failed to get aliyun_log pointer");
        return nullptr;
    }
    aliyun_log *aliyun = (aliyun_log*)token;
    
    // get the K-V pair count
    uint32_t length;
    status = napi_get_value_uint32(env, args[1], &length);
    if (napi_ok != status)
    {
        napi_throw_error(env, nullptr, "Failed to get log array length");
        return nullptr;
    }
    
    // make K-V array.the K-V length = pari count * 2
    char** key_values = (char**)malloc(length * 2 * sizeof(char*));
    for (uint32_t i = 0; i < length; i++) 
    {
        napi_value element;
        // key
        status = napi_get_element(env, args[2], i * 2, &element);
        if (napi_ok != status)
        {
            napi_throw_error(env, nullptr, "Failed to get array element");
            return nullptr;
        }
        
        size_t str_len;
        status = napi_get_value_string_utf8(env, element, nullptr, 0, &str_len);
        if (napi_ok != status)
        {
            napi_throw_error(env, nullptr, "Failed to get string length");
            return nullptr;
        }
        
        char* key = (char*)malloc(str_len + 1);
        status = napi_get_value_string_utf8(env, element, key, str_len + 1, &str_len);
        if (napi_ok != status)
        {
            napi_throw_error(env, nullptr, "Failed to get string");
            return nullptr;
        }
        key_values[i * 2] = key;
        
        // value
        status = napi_get_element(env, args[2], i * 2 + 1, &element);
        if (napi_ok != status)
        {
            napi_throw_error(env, nullptr, "Failed to get array element");
            return nullptr;
        }
        
        status = napi_get_value_string_utf8(env, element, nullptr, 0, &str_len);
        if (napi_ok != status)
        {
            napi_throw_error(env, nullptr, "Failed to get string length");
            return nullptr;
        }
        
        char* value = (char*)malloc(str_len + 1);
        status = napi_get_value_string_utf8(env, element, value, str_len + 1, &str_len);
        if (napi_ok != status)
        {
            napi_throw_error(env, nullptr, "Failed to get string");
            return nullptr;
        }
        key_values[i * 2 + 1] = value;
    }
    
    uint32_t ret = aliyun_log_add_log(aliyun, length, key_values);

    // dealloc k-k mem
    for (size_t i = 0; i < length; i++) {
        free(key_values[i * 2]);
        free(key_values[i * 2 + 1]);
    }
    free(key_values);

    napi_value result;
    status = napi_create_uint32(env, ret, &result);
    if (napi_ok != status)
    {
        napi_throw_error(env, nullptr, "Failed to create result");
        return nullptr;
    }
    return result;
}

static napi_value set_log_callback(napi_env env, napi_callback_info info)
{
    return aliyun_log_napi_set_callback(env, info);
}

static napi_value set_accesskey(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 4;
    size_t argc = 4;
    napi_value args[4] = {nullptr};
    
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (argc < requireArgc)
    {
        napi_throw_error(env, nullptr, "Wrong arguments. 3 arguments are expected.");    
        return nullptr;
    }
    
    // get the aliyun_log* pointer
    void *token;
    napi_status status = napi_get_value_external(env, args[0], &token);
    if (napi_ok != status)
    {
        napi_throw_error(env, nullptr, "Failed to get aliyun_log pointer");
        return nullptr;
    }
    aliyun_log *aliyun = (aliyun_log*)token;
    
    char* access_key_id;
    char* access_key_secret;
    char* access_key_token;
    
    _napi_get_value_string(env, args[1], &access_key_id);
    _napi_get_value_string(env, args[2], &access_key_secret);
    _napi_get_value_string(env, args[3], &access_key_token);
    
    aliyun_log_set_accesskey(aliyun, access_key_id, access_key_secret, access_key_token);
    
    return nullptr;
}

static napi_value set_topic(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (argc < requireArgc)
    {
        napi_throw_error(env, nullptr, "Wrong arguments. 2 arguments are expected.");    
        return nullptr;
    }
    
    // get the aliyun_log* pointer
    void *token;
    napi_status status = napi_get_value_external(env, args[0], &token);
    if (napi_ok != status)
    {
        napi_throw_error(env, nullptr, "Failed to get aliyun_log pointer");
        return nullptr;
    }
    aliyun_log *aliyun = (aliyun_log*)token;
    
    char* topic;
    _napi_get_value_string(env, args[1], &topic);
    if (NULL == topic) {
        return nullptr;
    }
    
    aliyun_log_set_topic(aliyun, topic);
    
    return nullptr;
}
static napi_value set_source(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (argc < requireArgc)
    {
        napi_throw_error(env, nullptr, "Wrong arguments. 2 arguments are expected.");    
        return nullptr;
    }
    
    // get the aliyun_log* pointer
    void *token;
    napi_status status = napi_get_value_external(env, args[0], &token);
    if (napi_ok != status)
    {
        napi_throw_error(env, nullptr, "Failed to get aliyun_log pointer");
        return nullptr;
    }
    aliyun_log *aliyun = (aliyun_log*)token;
    
    char* source;
    _napi_get_value_string(env, args[1], &source);
    if (NULL == source) {
        return nullptr;
    }
    
    aliyun_log_set_source(aliyun, source);
    
    return nullptr;
}
static napi_value add_tag(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 3;
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (argc < requireArgc)
    {
        napi_throw_error(env, nullptr, "Wrong arguments. 3 arguments are expected.");    
        return nullptr;
    }
    
    // get the aliyun_log* pointer
    void *token;
    napi_status status = napi_get_value_external(env, args[0], &token);
    if (napi_ok != status)
    {
        napi_throw_error(env, nullptr, "Failed to get aliyun_log pointer");
        return nullptr;
    }
    aliyun_log *aliyun = (aliyun_log*)token;
    
    char* tag_key;
    char* tag_value;
    _napi_get_value_string(env, args[1], &tag_key);
    _napi_get_value_string(env, args[2], &tag_value);
    if (NULL == tag_key || NULL == tag_value) {
        return nullptr;
    }
    
    aliyun_log_add_tag(aliyun, tag_key, tag_value);
    
    return nullptr;
}
static napi_value debuggable(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (argc < requireArgc)
    {
        napi_throw_error(env, nullptr, "Wrong arguments. 2 arguments are expected.");    
        return nullptr;
    }
    
    // get the aliyun_log* pointer
    void *token;
    napi_status status = napi_get_value_external(env, args[0], &token);
    if (napi_ok != status)
    {
        napi_throw_error(env, nullptr, "Failed to get aliyun_log pointer");
        return nullptr;
    }
    aliyun_log *aliyun = (aliyun_log*)token;
    
    int32_t debuggable;
    status = napi_get_value_int32(env, args[1], &debuggable);
    
    if (NULL == debuggable) {
        return nullptr;
    }
    
    aliyun_log_debuggable(aliyun, debuggable);
    
    return nullptr;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        { "createInstance", nullptr, create_instance, nullptr, nullptr, nullptr, napi_default, nullptr},
        { "destroyInstance", nullptr, destroy_instance, nullptr, nullptr, nullptr, napi_default, nullptr},
        { "addLog", nullptr, add_log, nullptr, nullptr, nullptr, napi_default, nullptr},
        { "setLogCallback", nullptr, set_log_callback, nullptr, nullptr, nullptr, napi_default, nullptr},
        { "setAccessKey", nullptr, set_accesskey, nullptr, nullptr, nullptr, napi_default, nullptr},
        { "setTopic", nullptr, set_topic, nullptr, nullptr, nullptr, napi_default, nullptr},
        { "setSource", nullptr, set_source, nullptr, nullptr, nullptr, napi_default, nullptr},
        { "addTag", nullptr, add_tag, nullptr, nullptr, nullptr, napi_default, nullptr},
        { "debuggable", nullptr, debuggable, nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);

    aliyun_log_set_os_http_function(napi_os_http_function);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version =1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "library",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterAliyun_log_harmony_sdkModule(void)
{
    napi_module_register(&demoModule);
}
