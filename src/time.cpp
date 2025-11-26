#pragma once

#include <time.h>
#include "logger.h"

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
    String result = tstring("%s %02d:%02d:%02d:%03lld", c_str(date), info->tm_hour, info->tm_min, info->tm_sec, milliseconds);
    
    return result;
}

global_variable Array <struct timespec> performance_timers = {0};

void push_performance_timer() { 
    struct timespec now;
    timespec_get(&now, TIME_UTC);
    performance_timers.append(now);
}

i64 pop_performance_timer_milliseconds() {
    struct timespec *start = performance_timers.pop();
    if (!start) {
        log("Poped performance timer while no timers on the list!", LOG_ERROR);
        return 0;   
    }

    struct timespec end;
    timespec_get(&end, TIME_UTC);
    
    i64 elapsed_sec  = end.tv_sec - start->tv_sec;
    i64 elapsed_nsec = end.tv_nsec - start->tv_nsec;
    
    // That's strange but that how it works, because nanoseconds are in range of one second, so in situation like 
    // 12 seconds and 300'000'000 nanoseconds as a start and a 
    // 13 seconds and 200'000'000 nanoseconds we cannot just subtract nanoseconds to get elapsed, so we improvise.
    if (elapsed_nsec < 0) {
        elapsed_sec -= 1;    
        elapsed_nsec += 1'000'000'000;
    }
    
    i64 result = elapsed_sec * 1000 + elapsed_nsec / 1'000'000;
    return result;
}
