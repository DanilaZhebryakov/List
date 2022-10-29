#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>
#include <time.h>


#include "time_utils.h"
#include "Console_utils.h"
#include "debug_utils.h"

#include "logging.h"

static const int max_print_args = 100;

FILE* initLogFile();

FILE* _logfile = initLogFile();

unsigned int _dump_file_counter = 0;

const time_t _program_run_time = time(nullptr);

void printGoodbyeMsg(){
    fprintf(_logfile, "Program exited\n");
    #ifdef LOG_USE_HTML
        fprintf(_logfile, LOG_HTML_FOOTER);
    #endif
}

FILE* initLogFile(){
    char filename[] = LOG_DIR_NAME "\\" LOG_BASE_NAME "00.00.0000_00:00" LOG_FILE_EXT;
    char* time_set_ptr = filename + strlen(LOG_DIR_NAME "\\" LOG_BASE_NAME);
    time_t time_s = time(nullptr);
    tm* tm_time = localtime(&time_s);
    sprintf(time_set_ptr, "%02d-%02d-%04d_%02d-%02d" LOG_FILE_EXT, tm_time->tm_mday, tm_time->tm_mon, tm_time->tm_year + 1900, tm_time ->tm_hour, tm_time->tm_min);

    FILE* logfile = fopen(filename, "a");
    if (errno == ENOENT){
        system("mkdir " LOG_DIR_NAME);
        logfile = fopen(filename, "a");
    }


    if (logfile == nullptr){
        perror("Error opening log file");
        return nullptr;
    }

    if (setvbuf(logfile, nullptr, _IONBF, 0) != 0){
        perror("Warning: can not set log file buffer");
    }

    #ifdef LOG_USE_HTML

        fprintf(logfile, LOG_HTML_HEADER , filename);
    #endif
    fprintf(logfile, "------------------------------------\n");
    time_t run_time = time(nullptr);
    fprint_time_date_short(logfile, run_time);

    fprintf(logfile, "Program started. Run id:%08X\n", run_time);
    fprintf(stderr , "Program started. Run id:%08X\n", run_time);

    atexit(printGoodbyeMsg);

    return logfile;
}

void printf_log(const char* format, ...){
    va_list args;
    va_start(args, max_print_args);
    setConsoleColor(stderr, COLOR_WHITE, COLOR_BLACK);
    vfprintf(_logfile, format , args);
    vfprintf( stderr , format , args);
    setConsoleColor(stderr, COLOR_DEFAULTT, COLOR_BLACK);
    va_end(args);
}

void error_log(const char* format, ...){
    va_list args;
    va_start(args, max_print_args);
    setLogColor((consoleColor)(COLOR_RED | COLOR_INTENSE));
    fprint_time_nodate(_logfile, time(nullptr));
    fprintf(_logfile, "[ERROR] ");
    fprintf( stderr , "[ERROR] ");
    vfprintf(_logfile, format , args);
    vfprintf( stderr , format , args);
    resetLogColor();
    va_end(args);
}

void warn_log(const char* format, ...){
    va_list args;
    va_start(args, max_print_args);
    setLogColor((consoleColor)(COLOR_YELLOW | COLOR_INTENSE));
    fprint_time_nodate(_logfile, time(nullptr));
    fprintf(_logfile, "[WARN] ");
    fprintf( stderr , "[WARN] ");
    vfprintf(_logfile, format , args);
    vfprintf( stderr , format , args);
    resetLogColor();
    va_end(args);
}
void info_log(const char* format, ...){
    va_list args;
    va_start(args, max_print_args);
    setConsoleColor(stderr, COLOR_WHITE, COLOR_BLACK);
    fprint_time_nodate(_logfile, time(nullptr));
    fprintf(_logfile, "[info] ");
    fprintf( stderr , "[info] ");
    vfprintf(_logfile, format , args);
    vfprintf( stderr , format , args);
    setConsoleColor(stderr, COLOR_DEFAULTT, COLOR_BLACK);
    va_end(args);
}

void debug_log(const char* format, ...){
    va_list args;
    va_start(args, max_print_args);
    setLogColor(COLOR_MAGENTA);
    fprint_time_nodate(_logfile, time(nullptr));
    fprintf(_logfile, "[DEBUG] ");
    fprintf( stderr , "[DEBUG] ");
    vfprintf(_logfile, format , args);
    vfprintf( stderr , format , args);
    resetLogColor();
    va_end(args);
}
void dumpData(const void* begin_ptr, size_t max_size){
    printf_log("     Raw data dump: (%ld total) [ ", max_size);
    size_t i = 0;
    while (i < max_size){
        if (!IsBadReadPtr((char*)begin_ptr + i, 1)){
            printf_log("%02X ", ((uint8_t*)begin_ptr)[i]);
        }
        else{
            printf_log("|access denied| ");
        }
        i++;
    }
    printf_log("]\n");
}

void embedNewDumpFile(char* filename_str, const char* filename_prefix, const char* filename_suffix, const char* html_type){
    sprintf(filename_str, LOG_DIR_NAME "\\%s_R%08X_N%d%s",
                          filename_prefix, _program_run_time, _dump_file_counter++, filename_suffix);
    info_log("External dump file created: %s\n", filename_str);
    fprintf(_logfile, "<%s src=%s width=1500>\n", html_type, filename_str + strlen(LOG_DIR_NAME) + 1);
}

void hline_log(){
    fprintf(stderr, "\n---------------------------\n");
    #ifdef LOG_USE_HTML
        fprintf(_logfile, "\n<hr>\n");
    #else
        fprintf(_logfile, "\n---------------------------\n");
    #endif
}

void setLogColor(consoleColor text_color, consoleColor background_color){
    setConsoleColor(stderr, text_color, background_color);
    #ifdef LOG_USE_HTML
        fprintf(_logfile, "<p style=\"");
        if(!(text_color       & COLOR_NOCHANGE)){
            fprintf(_logfile, "color: #%06x "  , consoleColorAsHex(text_color      ));
        }
        if(!(background_color & COLOR_NOCHANGE)){
            fprintf(_logfile, "bgcolor: #%06x ", consoleColorAsHex(background_color));
        }
        fprintf(_logfile, "\">");
    #endif
}

void resetLogColor(){
    setConsoleColor(stderr, COLOR_DEFAULTT, COLOR_BLACK);
    #ifdef LOG_USE_HTML
        fprintf(_logfile, "</p>");
    #endif
}

void header_log(const char* str){
    fprintf(stderr, "\n         >%s<\n", str);
    #ifdef LOG_USE_HTML
        fprintf(_logfile, "\n<h2>%s</h2>\n", str);
    #else
        fprintf(stderr, "\n         >%s<\n", str);
    #endif
}
