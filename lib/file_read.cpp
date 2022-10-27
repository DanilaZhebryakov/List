#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include "file_read.h"
#include "asserts.h"

#define EMPTY_TEXT ((Text){nullptr, nullptr, 0})

size_t getFileSize( FILE* file){
    assert_ret_err(file != nullptr, 0, EFAULT);
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file) + 1;
    rewind(file);
    return file_size;
}

void* readBinFile(const char* filename, size_t* len){
    assert_retnull(filename != nullptr);

    FILE* file = fopen(filename, "rb");
    if (file == nullptr) {
        fprintf(stderr, "File Not Found\n");
        return nullptr;
    }

    size_t file_size = getFileSize(file)-1;
    char* file_content = (char*)calloc(file_size, sizeof(char));
    assert_retnull(file_content != nullptr);

    size_t fread_val = fread( file_content, sizeof(char), file_size, file);
    fclose(file);
    if (fread_val != file_size){
        fprintf(stderr,"Error: fread error \nFile:%s \nLine:%d \nFunc:%s\nExpected %Iu bytes ,got %Iu. \nFunction returns nullptr\n",
                                            __FILE__,__LINE__, __PRETTY_FUNCTION__, file_size, fread_val);
        free(file_content);
        return nullptr;
    }

    *len = file_size;
    return file_content;
}

String readFile( FILE* file) {
    assert_ret_err(file != nullptr, ((String){nullptr, 0}), EFAULT);

    size_t file_size = getFileSize(file);
    char* file_content = (char*)calloc((file_size + 1), sizeof(char));
    assert_ret(file_content != nullptr, ((String){nullptr, 0}));
    size_t fread_val = fread( file_content, sizeof(char), file_size, file);
    if (errno != 0){
        fprintf(stderr,"Error: fread error \nFile:%s \nLine:%d \nFunc:%s\nExpected %Iu chars ,got %Iu. \nFunction returns nullptr\n",
                                            __FILE__,__LINE__, __PRETTY_FUNCTION__, file_size, fread_val);
        free(file_content);
        return {nullptr, 0};
    }

    file_content[fread_val] = '\0';
    return {file_content, fread_val};
}

void deleteText(Text* text){
    if (text == nullptr)
        return;

    if(text->lines != nullptr)
    free(text->lines);

    if(text->text_data != nullptr)
    free(text->text_data);

    text->lines     = nullptr;
    text->text_data = nullptr;
}

Text readFileLines(const char* filename){
    assert_ret_err(filename != nullptr, EMPTY_TEXT, EFAULT);

    FILE* input = fopen(filename, "r");
    if (input == nullptr) {
        fprintf(stderr, "File Not Found\n");
        return EMPTY_TEXT;
    }

    errno = 0;
    String file_content = readFile(input);
    fclose(input);
    if (file_content.chars == nullptr) {
        perror("Error while reading file");
        return EMPTY_TEXT;
    }

    size_t line_count = 0;
    errno = 0;
    String* lines = split(file_content, &line_count, '\n');
    if (lines  == nullptr) {
        perror("Unexpected error");
        return EMPTY_TEXT;
    }
    return (Text){lines, file_content.chars, line_count};
};

