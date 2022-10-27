#include <string.h>
#include "parseArg.h"

int parseArg(int argc, const char* argv[], const char* arg_to_find) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], arg_to_find) == 0){
            return i;
        }
    }
    return ARG_NOT_FOUND;
}
