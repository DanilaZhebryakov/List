#ifndef TIME_UTILS_H_INCLUDED
#define TIME_UTILS_H_INCLUDED

void fprint_mm_ss(FILE* file, time_t time);

void fprint_hh_mm_ss(FILE* file, time_t time);

void fprint_time_date_short(FILE* file, time_t time);

void fprint_time_nodate(FILE* file, time_t time);

#endif // TIME_UTILS_H_INCLUDED
