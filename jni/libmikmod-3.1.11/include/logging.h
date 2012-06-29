#ifndef _ANDROID_LOGGING_
#define _ANDROID_LOGGING_

#include <android/log.h>
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "MikMod", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,   "MikMod", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,    "MikMod", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,    "MikMod", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,   "MikMod", __VA_ARGS__)


#endif
