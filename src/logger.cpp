#pragma once
#include "logger.h"

const char* RED_TEXT = "\033[31m";
const char* GREEN_TEXT = "\033[32m";
const char* RESET_TEXT = "\033[0m"; 

void log_error_without_saving_to_file(String message) {
    printf(tprintf("%s%s%s\n", RED_TEXT, c_str(message), RESET_TEXT));
}

struct Log_File {
    b32 opened = false;
    String name = {0};
    FILE *file = NULL;        
};

Log_File log_file = {0};

b32 open_or_create_log_file() {
    if (log_file.opened) { 
        assert(log_file.file && "Log file marked as opened, but file pointer is NULL!");
        return false;
    }

    String directory_name = tstring("logs");
    make_directory_if_not_exists(directory_name);

    String name = tstring("%s/Log_%s.txt", c_str(directory_name), c_str(get_current_date_tstring()));
    
    log_file.file = fopen(c_str(name), "a");
    if (!log_file.file) {
        log_file.opened = false;
        log_error_without_saving_to_file(tstring("Failed to open log file! Name was: %s", c_str(name)));
        return false;
    }
    
    log_file.opened = true;   
    log_file.name = copy_string(name, HEAP_ALLOCATOR);
    
    return true;
}

void log_to_file(String message) {      
    if (!log_file.opened) {
        b32 r = open_or_create_log_file();
        if (!r) {
            log_error_without_saving_to_file(tstring("FAILED TO OPEN OR CREATE LOG FILE!"));
            return;
        }
    }
        
    assert(log_file.file);
    fprintf(log_file.file, c_str(message));
}

void game_log(const char *str) {
    Log_Message *new_log = debug.log_messages_short.append({0});
    str_copy(new_log->data, str);
    
    // Printing |'s just to see that we printing something if there's much of the same things going on.
    local_persist u64 count = 0;
    if (count++ & 1) sprintf(new_log->data, "%s |", new_log->data);
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

enum Log_Flags : u64 {
    LOG_ERROR        = 0x1, 
    PUSH_INDENTATION = 0x2,
    POP_INDENTATION  = 0x4,
};

i32 logs_indentation_count = 0;

void push_log_indentation() {
    logs_indentation_count += 1;    
}
void pop_log_indentation() {
    logs_indentation_count -= 1;
    
    log_if_false(logs_indentation_count >= 0, "Poped too mush log indentation! logs_indentation_count is %s", logs_indentation_count);
}

inline void log(String message, u64 flags) { 
    String_Builder builder = {.allocator = temp};
    
    for (i32 i = 0; i < logs_indentation_count; i++) {
        builder_append(&builder, tstring("\t"));    
    }
    
    builder_append(&builder, tstring("%s\n", c_str(message)));
    
    String console_message = make_string_from_builder(&builder, temp);
    
    if (flags & LOG_ERROR) {
        printf("%s%s%s", RED_TEXT, c_str(console_message), RESET_TEXT);
    } else {
        printf(c_str(console_message));
    }
    
    String log_message = {0};
    
    if (flags & LOG_ERROR) {
        log_message = tstring("%s [ERROR] %s", c_str(get_current_date_with_time_tstring()), c_str(console_message));
    } else {
        log_message = tstring("%s [INFO] %s", c_str(get_current_date_with_time_tstring()), c_str(console_message));
    }
    
    log_to_file(log_message);
    
    // Applying indentation changes afterwards on purpose.
    if (flags & PUSH_INDENTATION) {
        push_log_indentation();
    }
    if (flags & POP_INDENTATION) {
        pop_log_indentation();
    }
}
inline void log(const char *message, u64 flags) {
    log(tstring(message), flags);
}

inline void log_if_false(b32 expression, const char *message, ...) {
    if (!expression) {
        va_list args;
        va_start(args, message);
        log(tstring(message, args), LOG_ERROR);
        va_end(args);
    }
}

void unload_log_file() {
    if (log_file.opened) {
        assert(log_file.file && "Log file marked as opened, but file pointer is NULL!");
        fclose(log_file.file);               
    }
}
