#include <stdio.h>
#include <locale.h>
#include <assert.h>
#include <wchar.h>
#include <string.h>
#include <windows.h>

#include "Console_utils.h"

unsigned int consoleColorAsHex(consoleColor col){
    unsigned int res = 0;
    int col_val = (col & COLOR_INTENSE) ? 0xFF : 0x7f;

    res |= (col & COLOR_RED  ) ? (col_val << 16)  : 0;
    res |= (col & COLOR_GREEN) ? (col_val << 8 )  : 0;
    res |= (col & COLOR_BLUE ) ? (col_val << 0 )  : 0;
    return res;
}

bool setConsoleColor(FILE* console, consoleColor text_color, consoleColor background_color){
    WORD textAttribute = 0;
    textAttribute |= (text_color       & COLOR_RED     ) ? FOREGROUND_RED       : 0;
    textAttribute |= (text_color       & COLOR_GREEN   ) ? FOREGROUND_GREEN     : 0;
    textAttribute |= (text_color       & COLOR_BLUE    ) ? FOREGROUND_BLUE      : 0;
    textAttribute |= (text_color       & COLOR_INTENSE ) ? FOREGROUND_INTENSITY : 0;

    textAttribute |= (background_color & COLOR_RED     ) ? BACKGROUND_RED       : 0;
    textAttribute |= (background_color & COLOR_GREEN   ) ? BACKGROUND_GREEN     : 0;
    textAttribute |= (background_color & COLOR_BLUE    ) ? BACKGROUND_BLUE      : 0;
    textAttribute |= (background_color & COLOR_INTENSE ) ? BACKGROUND_INTENSITY : 0;

    if(console == stdin)
        return SetConsoleTextAttribute(GetStdHandle(STD_INPUT_HANDLE ), textAttribute);
    if(console == stdout)
        return SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), textAttribute);
    if(console == stderr)
        return SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE ), textAttribute);
    return 0;
}

static bool setConsoleFont(DWORD console){
    CONSOLE_FONT_INFOEX font = {sizeof(font)};
    if (!GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), false, &font)){
       return 0;
    }
    font.FontFamily = 0x36;
    font.dwFontSize.Y = 15;
    wcsncpy(font.FaceName, L"Consolas", LF_FACESIZE);
    return SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), false, &font);
}

void initConsole(){

    const unsigned int codepage_id = 1251; // ANSI
    SetConsoleCP      (codepage_id);
    SetConsoleOutputCP(codepage_id);
    setlocale (LC_ALL,     "RU");
    setlocale (LC_NUMERIC, "C" );

    setConsoleFont(STD_INPUT_HANDLE );
    setConsoleFont(STD_OUTPUT_HANDLE);
    setConsoleFont(STD_ERROR_HANDLE );
}

bool moveCursor(FILE* console, int dx, int dy){
    HANDLE WINAPI console_handle = 0;
    if     (console == stdin){
        console_handle = GetStdHandle(STD_INPUT_HANDLE);
    }
    else if(console == stdout){
        console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    else if(console == stderr){
        console_handle = GetStdHandle(STD_ERROR_HANDLE);
    }
    else{
        return 0;
    }

    _CONSOLE_SCREEN_BUFFER_INFO screen_info = {};
    GetConsoleScreenBufferInfo(console_handle, &screen_info);
    screen_info.dwCursorPosition.X += dx;
    screen_info.dwCursorPosition.Y += dy;
    return SetConsoleCursorPosition  (console_handle, screen_info.dwCursorPosition);
}


void createProgressBar(FILE* out, int total, int filled, const char* bar_string, consoleColor color_full, consoleColor color_empty){
    assert(strlen(bar_string) >= 5);
    fputc(bar_string[0], out);
    if (!(color_full & COLOR_NOCHANGE)){
        fflush(out);
        setConsoleColor(out, color_full, COLOR_BLACK);
    }
    for (int x = 0; x < filled; x++)
        fputc(bar_string[1], out);
    if (!(color_empty & COLOR_NOCHANGE)){
        fflush(out);
        setConsoleColor(out, color_empty, COLOR_BLACK);
    }


    if (filled != total)
        fputc(bar_string[2], out);
    for (int x = filled+1; x < total; x++)
        fputc(bar_string[3], out);

    fputc(bar_string[4], out);
    fflush(out);
}
void createNormalProgressBar(FILE* out, int total, int filled){
    createProgressBar(out, total, filled, "[=  ]", COLOR_GREEN, COLOR_DEFAULTT);
}

void createSimpleProgressBar(FILE* out, int total, int filled){
    createProgressBar(out, total, filled, "[=  ]", COLOR_NOCHANGE, COLOR_NOCHANGE);
}

