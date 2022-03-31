#pragma once

void __log(const char* level, const char* tag, const char* tex, ...);

#define ESP_LOGE(tag, text, ...) ESP_LOG("ERR", tag, text, __VA_ARGS__)
#define ESP_LOGW(tag, text, ...) ESP_LOG("WARN", tag, text, __VA_ARGS__)
#define ESP_LOGI(tag, text, ...) ESP_LOG("INFO", tag, text, __VA_ARGS__)
#define ESP_LOGD(tag, text, ...) ESP_LOG("DEBUG", tag, text, __VA_ARGS__)
#define ESP_LOGV(tag, text, ...) ESP_LOG("VERBOSE", tag, text, __VA_ARGS__)

#define ESP_LOG(level, tag, text, ...) __log(level, tag, text, __VA_ARGS__)
