#ifndef LOGGING_H_INCLUDED
#define LOGGING_H_INCLUDED
    #include <stdio.h>
    #include <stdlib.h>
    #include <time.h>
    #include <string.h>
    #include <stdarg.h>

    #include "Console_utils.h"
    #include "asserts.h"
    #include "time_utils.h"
    #include "debug_utils.h"

    extern FILE* _logfile;


    void printf_log(const char* format, ...);

    void perror_log_(const char* errmsg, const char* add_info);

    void error_log(const char* format, ...);

    void warn_log(const char* format, ...);

    void info_log(const char* format, ...);

    void debug_log(const char* format, ...);

    #ifdef perror_log
        #error redefinition of internal macro Error_log perror_log
    #endif
    #define perror_log(errmsg)  do {    \
        setConsoleColor(stderr, (consoleColor)(COLOR_RED | COLOR_INTENSE), COLOR_BLACK); \
        fprint_time_nodate(_logfile, time(nullptr));                                     \
        fprintf(_logfile, "[ERROR] %s :%s\n at: \nFile:%s \nLine:%d \nFunc:%s\n",        \
                  errmsg, strerror(errno), __FILE__, __LINE__, __PRETTY_FUNCTION__);     \
        fprintf(stderr  , "[ERROR] %s :%s\n at: \nFile:%s \nLine:%d \nFunc:%s\n",        \
                  errmsg, strerror(errno), __FILE__, __LINE__, __PRETTY_FUNCTION__);     \
        setConsoleColor(stderr, COLOR_DEFAULTT, COLOR_BLACK);\
    } while(0)

    #ifdef Error_log
        #error redefinition of internal macro Error_log
    #endif
    #define Error_log(format, ...) {                                                     \
        setConsoleColor(stderr, (consoleColor)(COLOR_RED | COLOR_INTENSE), COLOR_BLACK);   \
        fprint_time_nodate(_logfile, time(nullptr));                                       \
        fprintf(_logfile, "[ERROR]" format , __VA_ARGS__);                                 \
        fprintf(stderr  , "[ERROR]" format , __VA_ARGS__);                                 \
        fprintf(_logfile, " at: \nFile:%s \nLine:%d \nFunc:%s\n",                          \
                   __FILE__, __LINE__, __PRETTY_FUNCTION__);       \
        fprintf(stderr  , " at: \nFile:%s \nLine:%d \nFunc:%s\n",                          \
                   __FILE__, __LINE__, __PRETTY_FUNCTION__);       \
        setConsoleColor(stderr, COLOR_DEFAULTT, COLOR_BLACK);      \
    }

    #ifdef assert_log
        #error redefinition of internal macro assert_log
    #endif
    #ifndef NDEBUG
        #define assert_log(cond)                                  \
        if(!(cond)){                                                 \
            setConsoleColor(stderr, COLOR_WHITE, COLOR_RED);           \
            fprint_time_nodate(_logfile, time(nullptr));               \
            fprintf(_logfile, "[ASSERT]" #cond);                       \
            fprintf(stderr  , "[ASSERT]" #cond);                       \
            fprintf(_logfile, " at: \nFile:%s \nLine:%d \nFunc:%s\n",  \
                       __FILE__, __LINE__, __PRETTY_FUNCTION__);       \
            fprintf(stderr  , " at: \nFile:%s \nLine:%d \nFunc:%s\n",  \
                       __FILE__, __LINE__, __PRETTY_FUNCTION__);       \
            setConsoleColor(stderr, COLOR_DEFAULTT, COLOR_BLACK);      \
            exit(EXIT_FAILURE);                                        \
        }
    #else
        #define assert_log(cond) ;
    #endif

    #ifdef $log
        #error redefinition of internal macro $log
    #endif
    #define $log(operation) {                                  \
    debug_log(#operation " at " __FILE__ ": %d\n", __LINE__);  \
    operation;                                                 \
    }

    void dumpData(const void* begin_ptr, size_t max_size);

#endif // LOGGING_H_INCLUDED
