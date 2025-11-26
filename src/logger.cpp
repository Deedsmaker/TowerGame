#pragma once
#include "logger.h"

const char* RED_TEXT = "\033[31m";
const char* GREEN_TEXT = "\033[32m";
const char* RESET_TEXT = "\033[0m"; 

void game_log(const char *str) {
    Log_Message *new_log = debug.log_messages_short.append({0});
    str_copy(new_log->data, str);
    
    // Printing |'s just to see that we printing something if there's much of the same things going on.
    local_persist u64 count = 0;
    if (count++ % 2) sprintf(new_log->data, "%s |", new_log->data);
    else           sprintf(new_log->data, "%s |||||", new_log->data);
    
    new_log->birth_time = core.time.app_time;
}
inline void game_log(String string) {
    game_log(c_str(string));
}

inline void game_log(f32 value) {
    game_log(tprintf("%f", value));
}
inline void game_log(Vector2 value) {
    game_log(tprintf("{%f, %f}", value.x, value.y));
}

inline void log_error(String message) {
    printf(tprintf("%s%s%s", RED_TEXT, c_str(message), RESET_TEXT));
}

inline void log_if_false(bool expression, String message) {
    if (!expression) {
        log_error(message);
    }
}
