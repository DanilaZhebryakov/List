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

    #ifndef LOG_BASE_NAME
        #define LOG_BASE_NAME ""
    #endif
    #ifndef LOG_DIR_NAME
        #define LOG_DIR_NAME "logs"
    #endif
    #ifndef LOG_FILE_EXT
        #define LOG_FILE_EXT ".log"
    #endif

    #define LOG_USE_HTML

    #ifndef LOG_HTML_HEADER
        #define LOG_HTML_HEADER     \
            "<head>\n"              \
            "<title>%s</title>\n"   \
            "<style>\n"             \
            "body {\n"              \
                "background: #111;\n"\
                "color: #fff;\n"    \
            "}\n"                   \
            "</style>\n"            \
            "</head>\n"             \
            "<body>\n"              \
            "<pre>\n"

    #endif

    #ifndef LOG_HTML_FOOTER
        #define LOG_HTML_FOOTER \
            "</pre>\n"          \
            "</body>\n"
    #endif

    extern FILE* _logfile;

    extern unsigned int dump_file_counter;

    extern const time_t program_run_time;

    void printf_log(const char* format, ...);

    void perror_log_(const char* errmsg, const char* add_info);

    void error_log(const char* format, ...);

    void warn_log(const char* format, ...);

    void info_log(const char* format, ...);

    void debug_log(const char* format, ...);

    void embedNewDumpFile(char* filename_str, const char* filename_prefix, const char* filename_suffix, const char* html_type);

    void setLogColor(consoleColor text_color, consoleColor background_color = (consoleColor)(COLOR_BLACK | COLOR_NOCHANGE));

    void resetLogColor();

    void hline_log();

    void header_log(const char* str);

    #ifdef perror_log
        #error redefinition of internal macro Error_log perror_log
    #endif
    #define perror_log(errmsg)  do {    \
        setLogColor((consoleColor)(COLOR_RED | COLOR_INTENSE)); \
        fprint_time_nodate(_logfile, time(nullptr));                                     \
        fprintf(_logfile, "[ERROR] %s :%s\n at: \nFile:%s \nLine:%d \nFunc:%s\n",        \
                  errmsg, strerror(errno), __FILE__, __LINE__, __PRETTY_FUNCTION__);     \
        fprintf(stderr  , "[ERROR] %s :%s\n at: \nFile:%s \nLine:%d \nFunc:%s\n",        \
                  errmsg, strerror(errno), __FILE__, __LINE__, __PRETTY_FUNCTION__);     \
        resetLogColor();\
    } while(0)

    #ifdef Error_log
        #error redefinition of internal macro Error_log
    #endif
    #define Error_log(format, ...) {                                                     \
        setLogColor((consoleColor)(COLOR_RED | COLOR_INTENSE));   \
        fprint_time_nodate(_logfile, time(nullptr));                                       \
        fprintf(_logfile, "[ERROR] " format , __VA_ARGS__);                                 \
        fprintf(stderr  , "[ERROR] " format , __VA_ARGS__);                                 \
        fprintf(_logfile, " at: \nFile:%s \nLine:%d \nFunc:%s\n",                          \
                   __FILE__, __LINE__, __PRETTY_FUNCTION__);       \
        fprintf(stderr  , " at: \nFile:%s \nLine:%d \nFunc:%s\n",                          \
                   __FILE__, __LINE__, __PRETTY_FUNCTION__);       \
        resetLogColor();      \
    }

    #ifdef assert_log
        #error redefinition of internal macro assert_log
    #endif
    #ifndef NDEBUG
        #define assert_log(cond)                                  \
        if(!(cond)){                                                 \
            setLogColor( COLOR_WHITE, COLOR_RED);           \
            fprint_time_nodate(_logfile, time(nullptr));               \
            fprintf(_logfile, "[ASSERT]" #cond);                       \
            fprintf(stderr  , "[ASSERT]" #cond);                       \
            fprintf(_logfile, " at: \nFile:%s \nLine:%d \nFunc:%s\n",  \
                       __FILE__, __LINE__, __PRETTY_FUNCTION__);       \
            fprintf(stderr  , " at: \nFile:%s \nLine:%d \nFunc:%s\n",  \
                       __FILE__, __LINE__, __PRETTY_FUNCTION__);       \
            resetLogColor();      \
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
