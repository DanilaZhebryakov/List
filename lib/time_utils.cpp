#include <stdio.h>
#include <time.h>

void fprint_mm_ss(FILE* file, time_t time){
    fprintf(file, "%02Id:%02Id", time/60, time%60);
}

void fprint_hh_mm_ss(FILE* file, time_t time){
    fprintf(file, "%02Id:%02Id:%02Id", (time/3600), (time/60)%60 , (time%60));
}

void fprint_time_date_short(FILE* file, time_t time){
    tm* tm_time = localtime(&time);
    fprintf(file, "[%02d:%02d:%02d %02d.%02d]", tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, tm_time->tm_mday, tm_time->tm_mon);
}

void fprint_time_nodate(FILE* file, time_t time){
    tm* tm_time = localtime(&time);
    fprintf(file, "[%02d:%02d:%02d]", tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
}
