//
// Created on 2023/12/22.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "aliyun_log.h"
#include "aliyun-log-c-sdk/log_producer_config.h"
#include "aliyun-log-c-sdk/log_producer_client.h"
#include "inner_log.h"
#include "log_http_interface.h"
#include <sys/sysinfo.h>

static aliyun_log_send_done_function _send_done_function;

static unsigned int _global_server_time;
static unsigned int _global_last_server_up_time;
static unsigned int _set_get_time_unix_func()
{
    unsigned int server_time = _global_server_time;
    unsigned int last_server_up_time = _global_last_server_up_time;
    
    if (0 == server_time || 0 == last_server_up_time)
    {
        return (unsigned int) time(NULL);
    }
    
    struct sysinfo info;
    sysinfo(&info);
    unsigned int now_up_time = info.uptime;
    
    if (now_up_time < last_server_up_time)
    {
        return now_up_time + server_time;
    }
    
    return (unsigned int)(now_up_time - last_server_up_time + server_time);
}

void _internal_on_log_send_done(
    const char* config_name, 
    log_producer_result result, 
    size_t log_bytes, 
    size_t compressed_bytes, 
    const char* req_id, 
    const char* error_message, 
    const unsigned char* raw_buffer,
    void* user_param)
{
    if (NULL == user_param)
    {
        return;
    }
    
    if (NULL == _send_done_function)
    {
        return;
    }
    
    _send_done_function(config_name, result, log_bytes, compressed_bytes, error_message, user_param);
}

void aliyun_log_update_server_time(unsigned int time)
{
    _global_server_time = time;
    
    struct sysinfo info;
    sysinfo(&info);
    _global_last_server_up_time = info.uptime;
}

aliyun_log* aliyun_log_create(
    const char* endpoint,
    const char* project,
    const char* logstore,
    const char* access_key_id,
    const char* access_key_secret,
    const char* access_key_token)
{
    if (log_producer_env_init() != LOG_PRODUCER_OK) 
    {
        return NULL; 
    }

    aos_log_set_level(AOS_LOG_WARN);

    log_producer_config *config = create_log_producer_config();
    log_producer_config_set_source(config, "HarmonyOS");
    log_producer_config_set_send_thread_count(config, 1);
    log_producer_config_set_packet_timeout(config, 3000);
    log_producer_config_set_packet_log_count(config, 1024);
    log_producer_config_set_packet_log_bytes(config, 1024*1024);
    log_producer_config_set_drop_delay_log(config, 0);
    log_producer_config_set_drop_unauthorized_log(config, 0);
    log_producer_config_set_compress_type(config, 1);
    
    log_producer_config_set_endpoint(config, endpoint);
    log_producer_config_set_project(config, project);
    log_producer_config_set_logstore(config, logstore);
    
    log_set_get_time_unix_func(_set_get_time_unix_func);
    
    if (NULL != access_key_token)
    {
        log_producer_config_reset_security_token(config, access_key_id, access_key_secret, access_key_token);
    }
    else
    {
        log_producer_config_set_access_id(config, access_key_id);
        log_producer_config_set_access_key(config, access_key_secret);
    }
    
    log_producer_config_set_packet_log_bytes(config, 4 * 1024 * 1024);
    log_producer_config_set_packet_log_count(config, 4096);
    log_producer_config_set_packet_timeout(config, 3000);
    log_producer_config_set_max_buffer_limit(config, 32 * 1024 * 1024);
    
    aliyun_log* log_token = (aliyun_log*)malloc(sizeof(aliyun_log));
    
    log_producer* producer = create_log_producer(config, _internal_on_log_send_done, log_token);
    log_producer_client* client = get_log_producer_client(producer, NULL);
    
    log_token->config = config;
    log_token->client = client;
    log_token->producer = producer;
    
    return log_token;
}

void aliyun_log_destroy(aliyun_log* aliyun_log)
{
    if (NULL == aliyun_log || NULL == aliyun_log->producer)
    {
        return;
    }
    
    destroy_log_producer(aliyun_log->producer);
    free(aliyun_log);
}

void aliyun_log_set_send_done_function(aliyun_log_send_done_function send_done_function)
{
    _send_done_function = send_done_function;
}

void aliyun_log_set_os_http_function(int (*f)(const char *url,
                                              char ***header_array,
                                              int header_count,
                                              const void *data,
                                              int data_len,
                                              post_log_result *http_response))
{
    log_set_http_post_func(f);
}

uint32_t aliyun_log_add_log(aliyun_log* aliyun_log, uint32_t length, char** key_values)
{
	if (NULL == aliyun_log)
	{
		return LOG_PRODUCER_INVALID;
	}

	char** keys = (char**)malloc(sizeof(char*) * length);
	int32_t* keys_length = (int32_t*)malloc(sizeof(int32_t) * length);
	char** values = (char**)malloc(sizeof(char*) * length);
	int32_t* values_length = (int32_t*)malloc(sizeof(int32_t) * length);
	for (int i = 0; i < length; i++)
	{
		keys[i] = key_values[2 * i];
		keys_length[i] = strlen(keys[i]);

		values[i] = key_values[2 * i + 1];
		values_length[i] = strlen(values[i]);
	}

    log_producer_result result = log_producer_client_add_log_with_len_time_int32(
        (log_producer_client *)aliyun_log->client,
        0,length, keys, keys_length, values, values_length, 0
    );
//	log_producer_result result = log_producer_client_add_log_with_len((log_producer_client*)aliyun_log->client, length, keys, keys_length, values, values_length, 0);

    free(keys);
    free(keys_length);
    free(values);
    free(values_length);

    return result;
}

void aliyun_log_set_accesskey(
    aliyun_log* aliyun_log,
    const char* access_key_id, 
    const char* access_key_secret, 
    const char* access_key_token)
{
    if (NULL == aliyun_log)
    {
        return;
    }
    
    log_producer_config *config = (log_producer_config *) aliyun_log->config;
    
    if (NULL == access_key_id || NULL == access_key_secret)
    {
        return;
    }
    
    if (NULL == access_key_token)
    {
        log_producer_config_set_access_id(config, access_key_id);
        log_producer_config_set_access_key(config, access_key_secret);
    }
    else
    {
        log_producer_config_reset_security_token(
            config, 
            access_key_id, 
            access_key_secret, 
            access_key_token
        );
    }
}

void aliyun_log_set_topic(aliyun_log *aliyun_log, const char* topic)
{
    if (NULL == aliyun_log)
    {
        return;
    }
    
    if (NULL == topic)
    {
        return;
    }
    
    log_producer_config *config = (log_producer_config *) aliyun_log->config;
    log_producer_config_set_topic(config, topic);
}           

void aliyun_log_set_source(aliyun_log *aliyun_log, const char* source)
{
    if (NULL == aliyun_log)
    {
        return;
    }
    
    if (NULL == source)
    {
        return;    
    }
    
    log_producer_config *config = (log_producer_config *) aliyun_log->config;
    log_producer_config_set_source(config, source);
}

void aliyun_log_add_tag(aliyun_log *aliyun_log, const char* key, const char* value)
{
    if (NULL == aliyun_log)
    {
        return;
    }
    
    if (NULL == key || NULL == value)
    {
        return;
    }
    
    log_producer_config *config = (log_producer_config *) aliyun_log->config;
    log_producer_config_add_tag(config, key, value);
}

void aliyun_log_debuggable(aliyun_log *aliyun_log, int32_t debuggable)
{
    if (NULL == aliyun_log)
    {
        return;
    }
    
    aos_log_set_level(debuggable ? AOS_LOG_TRACE : AOS_LOG_WARN);
}