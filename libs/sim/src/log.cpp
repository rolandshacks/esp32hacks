//
// ESP32 Logging
//

#include <cstdio>
#include <stdarg.h>

void __log(const char* level, const char* tag, const char* format, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 4096, format, args);
    va_end (args);

    printf("%s %s: %s\n", level, tag, buffer);
}
