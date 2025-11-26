#pragma once
#include "logger.h"

void log_short(const char *str) {
    Log_Message *new_log = debug.log_messages_short.append({0});
    str_copy(new_log->data, str);
    
    // Printing |'s just to see that we printing something if there's much of the same things going on.
    local_persist u64 count = 0;
    if (count++ % 2) sprintf(new_log->data, "%s |", new_log->data);
    else           sprintf(new_log->data, "%s |||||", new_log->data);
    
    new_log->birth_time = core.time.app_time;
}
inline void log_short(String string) {
    log_short(c_str(string));
}

inline void log_short(f32 value) {
    log_short(tprintf("%f", value));
}
inline void log_short(Vector2 value) {
    log_short(tprintf("{%f, %f}", value.x, value.y));
}

inline void log_if_false(bool expression, String message) {
    if (!expression) {
        printf(c_str(message));
    }
}
