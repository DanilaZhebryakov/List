//! @file file_read.h
#ifndef FILE_READ_H_INCLUDED
#define FILE_READ_H_INCLUDED

#include "String.h"

struct Text{
    String* lines;
    char* text_data;
    size_t length;
};

void deleteText(Text* text);

//! gets size of file (number of characters in it)
//! @param [in] file input file
//! @return size
//! @note returns 0 in case of error
//! @note works incorrectly in some OSs because of different text format
size_t getFileSize( FILE* file);

//! reads file as string
//! @param [in] file input file
//! @param [out] file_size size of file
//! @return file content as string
//! @note function allocates memory for its output. You need to free() it
//! @note returns nullptr in case of error
String readFile( FILE* file);

Text readFileLines(const char* filename);

void* readBinFile(const char* filename, size_t* len);

#endif
