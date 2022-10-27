#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "String.h"
#include "asserts.h"

bool isTextChar(char c) {
    return(c >= 'a' || c < 0);
}

bool isTextLine(char* str) {
    while (*str != '\0') {
        if (isTextChar(*str)) {
            return true;
        }
        str++;
    }
    return false;
}

int Strcmp(const String* a, const String* b) {
    const char* a_ptr = a->chars;
    const char* b_ptr = b->chars;
    a_ptr--;
    b_ptr--;

    do {
        a_ptr++;
        b_ptr++;
        while (*a_ptr != '\0' && isIncomparable(*a_ptr))
            a_ptr++;
        while (*b_ptr != '\0' && isIncomparable(*b_ptr))
            b_ptr++;
    } while(*a_ptr != '\0' && *a_ptr == *b_ptr);

    return *a_ptr - *b_ptr;
}

bool isIncomparable(char c){
    return (c >= '\0' && c < 'A') || (c > 'Z' && c < 'a') || (c == '—') || (c == '»');
}
int Strcmp_backw(const String* a, const String* b) {
    const char* a_ptr = a->chars + a->length;
    const char* b_ptr = b->chars + b->length;
    a_ptr++;
    b_ptr++;

    do {
        a_ptr--;
        b_ptr--;
        while (a_ptr >= a->chars && isIncomparable(*a_ptr))
            a_ptr--;
        while (b_ptr >= b->chars && isIncomparable(*b_ptr))
            b_ptr--;
    } while(a_ptr >= a->chars && b_ptr >= b->chars && *a_ptr == *b_ptr);
    if (a_ptr < a->chars){
        if (b_ptr < b->chars){
            return 0;
        }
        return -1;
    }

    if (b_ptr < b->chars){
        return 1;
    }

    return *a_ptr - *b_ptr;
}

int Strncmp_backw(const String* a, const String* b, int n) {
    //printf("Compare: %s %s\n", a->chars, b->chars);
    const char* a_ptr = a->chars + a->length;
    const char* b_ptr = b->chars + b->length;
    a_ptr++;
    b_ptr++;
    n++;
    do {
        n--;
        a_ptr--;
        b_ptr--;
        while (a_ptr >= a->chars && isIncomparable(*a_ptr))
            a_ptr--;
        while (b_ptr >= b->chars && isIncomparable(*b_ptr))
            b_ptr--;
        //printf("Char %d; %c/%c\n", n, *a_ptr, *b_ptr);
    } while(n > 0 && a_ptr >= a->chars && b_ptr >= b->chars && *a_ptr == *b_ptr);

    if(n == 0){
        return 0;
    }

    if (a_ptr < a->chars){
        if (b_ptr < b->chars){
            return 0;
        }
        return -1;
    }

    if (b_ptr < b->chars){
        return 1;
    }

    return *a_ptr - *b_ptr;
}


String* split( String str,size_t* nlines, char separator) {
    assert_ret_err( str.chars != nullptr, nullptr, EFAULT);

    *nlines = countLines(str.chars, separator);
    assert_retnull(nlines != 0);

    String* lines = (String*)calloc( *nlines, sizeof(String));
    assert_retnull(lines != nullptr);

    lines[0].chars = str.chars;
    size_t line_index = 1, line_size = 0;
    for (size_t i = 0; str.chars[i] != '\0'; i++) {
        if (str.chars[i] == separator) {
            str.chars[i] = '\0';
            lines[line_index].chars = &str.chars[i+1];
            lines[line_index-1].length = line_size;

            line_size = 0;
            line_index++;
        }
        else line_size++;
    }
    lines[line_index-1].length = line_size;

    return lines;
}

size_t countLines(char* str, char separator) {
    assert_ret_err(str != nullptr, 0, EFAULT);
    size_t nlines = 1;
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] == separator) {
            nlines++;
        }
    }
    return nlines;
}

size_t lstrip(String* str, char target_char) {
    assert_ret(target_char != '\0', 0);
    size_t char_count = 0;
    while (*(str->chars) == target_char) {
        str->chars++;
        str->length--;
        char_count++;
    }
    return char_count;
}

void nullifyString(String* str){
    *(str->chars) = '\0';
    str->length = 0;
}

int countSlogsLast(String str){
    int res = 0;
    char* ptr = str.chars + str.length;
    while (ptr >= str.chars && *ptr != ' ') {
        if(strchr("σεϋΰξύÿθώ", *ptr) != nullptr){
            res++;
        }
    }
}

