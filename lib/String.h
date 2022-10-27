//! @file String.h
//! contains String struct and some helper functions

#ifndef STRING_H_INCLUDED
#define STRING_H_INCLUDED

//! structure which stores string and its length as int
struct String{
    char* chars;
    size_t length;
};

bool isIncomparable(char c);

bool isTextChar(char c);

bool isTextLine(char* str);

//! version of strcmp which compares strings right-to-left instead of left-to-right and uses String* as input
int Strcmp_backw(const String* a, const String* b);

int Strncmp_backw(const String* a, const String* b, int n);

//! version of strcmp which uses String* as input
int Strcmp (const String* a, const String* b);

//! removes all target_char characters from beginning of string (WARNING: function moves the beginning of string. Be careful if using free())
//! @param str pointer to input string
//! @param target_char character to remove
//! @return number of character removed
size_t lstrip(String* str, char target_char);

//! splits string by given separator character and outputs it as array of strings
//! @param [in] str input_string
//! @note this function changes input_string (replaces all separators with '\0's)
//! @param [out] nlines size of the output array
//! @param [in] separator separator character to split string by
//! @return parts of input string as array of strings
//! @note function allocates memory for its output. You need to free() it
//! @note returns nullptr in case of error
String* split(String str,size_t* nlines, char separator);

//! counts [separator] terminated lines (+ 1 for '\0' terminated) in given string
//! @param [in] str input string
//! @param [in] separator configurable line break character to count
//! @return number of lines
//! @note returns 0 in case of error
size_t countLines(char* str, char separator);

//! replaces string with empty string
void nullifyString(String* str);

//! counts slogs in last word in string
int countSlogsLast(String str);

#endif // STRING_H_INCLUDED
