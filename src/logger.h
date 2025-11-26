#pragma once

inline void log_if_false(b32 expression, const char *message, ...);
inline void log(String message, u64 flags = 0);
inline void log(const char *message, u64 flags = 0);

