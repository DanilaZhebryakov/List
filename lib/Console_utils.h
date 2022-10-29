#ifndef CONSOLE_UTILS_H_INCLUDED
#define CONSOLE_UTILS_H_INCLUDED

enum consoleColor {
    COLOR_BLACK   = 0b000,
    COLOR_RED     = 0b100,
    COLOR_GREEN   = 0b010,
    COLOR_BLUE    = 0b001,
    COLOR_YELLOW  = 0b110,
    COLOR_CYAN    = 0b011,
    COLOR_MAGENTA = 0b101,
    COLOR_WHITE   = 0b111,

    COLOR_INTENSE   = 0b1000,
    COLOR_DEFAULTT  = 0b1111,
    COLOR_REDI      = 0b1100,
    COLOR_NOCHANGE  = 0b10000
};
unsigned int consoleColorAsHex(consoleColor col);

bool setConsoleColor(FILE* console, consoleColor text_color, consoleColor background_color);

void initConsole();

bool moveCursor(FILE* console, int dx, int dy);

void createProgressBar(FILE* out, int total, int filled, const char* bar_string, consoleColor color_full, consoleColor color_empty);

void createNormalProgressBar(FILE* out, int total, int filled);

void createSimpleProgressBar(FILE* out, int total, int filled);

#endif // CONSOLE_UTILS_H_INCLUDED
