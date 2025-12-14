#pragma once

enum Log_Flags : u64 {
    LOG_ERROR        = 0x1, 
    PUSH_INDENTATION = 0x2,
    POP_INDENTATION  = 0x4,
    LOG_WARNING      = 0x8,
};

inline void log_if_false(b32 expression, const char *message, ...);
inline void log(String message, u64 flags = 0);
inline void log(const char *message, u64 flags = 0);

