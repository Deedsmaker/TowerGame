#pragma once

#include <time.h>

inline f32 get_seconds_since_window_open_f32() {
    return GetTime();
}

struct tm *get_time_info(time_t *second = NULL) {
    struct tm *local_time_info;
    
    time_t time_raw = time(NULL);
    local_time_info = localtime(&time_raw);
    
    return local_time_info;
}

String get_current_date_tstring() {
    auto time_info = get_time_info();   
    String result = tstring("%04d-%02d-%02d", time_info->tm_year + 1900, time_info->tm_mon + 1, time_info->tm_mday);
    return result;
}

i64 get_current_milliseconds_i64() {
    struct timespec info;
    
    timespec_get(&info, TIME_UTC);
    
    // i64 nanoseconds = info.tv_sec * (i64)1'000'000'000 + info.tv_nsec;
    // i64 milliseconds = nanoseconds / 1'000'000;
    
    i64 milliseconds = info.tv_nsec / 1'000'000;
    
    return milliseconds;
}

String get_current_date_with_time_tstring() {
    String date = get_current_date_tstring();   
    
    auto info = get_time_info();
    
    i64 milliseconds = get_current_milliseconds_i64();
    String result = tstring("%s_%02d:%02d:%02d:%03lld", c_str(date), info->tm_hour, info->tm_min, info->tm_sec, milliseconds);
    
    return result;
}
