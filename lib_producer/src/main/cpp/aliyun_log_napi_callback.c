//
// Created on 2024/2/1.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
#include "aliyun_log_napi_callback.h"
#include "aliyun_log_napi_utils.h"
#include "aliyun_log.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

typedef struct _work_data {
    const char *logstore_name;
    uint32_t result_code;
    size_t log_byte;
    size_t compressed_bytes;
    const char* error_message;
    void *user_param;
} work_data;

static napi_callback_reference *callback_ref;

static void work_callback(uv_work_t *work) 
{
}

static void work_after_callback(uv_work_t *work, int status) 
{
    napi_env env = callback_ref->env;
    napi_handle_scope scope = NULL;
    napi_open_handle_scope(env, &scope);
    if (scope == NULL) {
        return;
    }

    // call js method
    napi_value callback;
    napi_get_reference_value(callback_ref->env, callback_ref->callback_ref, &callback);
    napi_value thiz;
    napi_get_reference_value(callback_ref->env, callback_ref->this_ref, &thiz);
    napi_value js_method;
    napi_get_named_property(callback_ref->env, callback, "onLogCallback", &js_method);

    // work data
    work_data *data = (work_data *) work->data;
    napi_value log_store;
    if (!aliyun_log_napi_create_string_utf8(env, &data->logstore_name, strlen(data->logstore_name), &log_store))
    {
        return;    
    }
    napi_value result_code;
    if (!aliyun_log_napi_create_int32(env, data->result_code, &result_code))
    {
        return;
    }
    napi_value log_byte;
    if (!aliyun_log_napi_create_int32(env, data->log_byte, &log_byte))
    {
        return;
    }
    napi_value compressed_bytes;
    if (!aliyun_log_napi_create_int32(env, data->compressed_bytes, &compressed_bytes))
    {
        return;
    }
    napi_value error_message;
    if (!aliyun_log_napi_create_string_utf8(env, &data->error_message, strlen(data->error_message), &error_message))
    {
        return;
    }
    
    napi_value argv[] = {
        log_store,
        result_code,
        log_byte,
        compressed_bytes,
        error_message
    };
    napi_value callback_result = NULL;
    napi_call_function(callback_ref->env, thiz, js_method, 5, argv, &callback_result);

    napi_close_handle_scope(env, scope);

    if (work != NULL) {
        free((void *)data->logstore_name);
        free((void *)data->error_message);
        free(work->data);

        free(work);
    }
}

void aliyun_log_napi_send_done_function(
    const char *config_name,
    uint32_t result,
    size_t log_byte,
    size_t compressed_bytes,
    const char *error_message,
    void *user_param) 
{
    struct uv_loop_s *loop = NULL;
    napi_get_uv_event_loop(callback_ref->env, &loop);

    work_data *data = (work_data *)malloc(sizeof(work_data));
    data->logstore_name = strdup(config_name);
    data->result_code = result;
    data->log_byte = log_byte;
    data->compressed_bytes = compressed_bytes;
    data->error_message = strdup(error_message);
    data->user_param = user_param;

    uv_work_t *work = (uv_work_t *)malloc((sizeof(uv_work_t)));
    work->data = (void *)data;

    uv_queue_work(
        loop,
        work,
        work_callback,
        // using callback function back to JS thread
        work_after_callback
    );
}

napi_value aliyun_log_napi_set_callback(napi_env env, napi_callback_info info) 
{
    size_t requireArgc = 1;
    size_t argc = 1;
    napi_value args[1] = {NULL};
    napi_value this_arg;

    napi_get_cb_info(env, info, &argc, args, &this_arg, NULL);
    if (argc < requireArgc) {
        napi_throw_error(env, NULL, "Wrong arguments. 1 arguments are expected.");
        return NULL;
    }

    if (NULL == callback_ref) {
        callback_ref = (napi_callback_reference *)malloc(sizeof(napi_callback_reference));
    }

    napi_create_reference(env, args[0], 1, &callback_ref->callback_ref);
    napi_create_reference(env, this_arg, 1, &callback_ref->this_ref);
    callback_ref->env = env;

    napi_value result;
    napi_status status = napi_create_uint32(env, 1, &result);
    if (napi_ok != status) {
        napi_throw_error(env, NULL, "Failed to create result");
        return NULL;
    }

    aliyun_log_set_send_done_function(aliyun_log_napi_send_done_function);

    return result;
}