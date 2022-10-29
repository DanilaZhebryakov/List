#include "debug_utils.h"

void printBaseError_log(baseError_t err){
    if(err & ERR_BADOP){
        printf_log("     (BAD) Invalid operation\n");
    }
    if(err & ERR_INTERR){
        printf_log("     (BAD) Internal error\n");
    }
    if(err & ERR_NULL){
        printf_log("     (BAD) Null pointer argument\n");
    }
    if(err & ERR_BAD){
        printf_log("     (BAD) Bad pointer argument\n");
    }
    if(err & ERR_DEAD){
        printf_log("     (BAD) Already destructed\n");
    }
    if(err & ERR_CORRUPT){
        printf_log("     (BAD) Data corrupted\n");
    }
    if(err & ERR_ERRUNK){
        printf_log("     (BAD) Unknown error[BUG]\n");
    }
}

void printVarError_log(varError_t err){
    printBaseError_log((baseError_t) err);
    if(err & VAR_BADSTATE){
        printf_log("     (BAD) Structure in invalid state\n");
    }
    if(err & VAR_DATA_NULL){
        printf_log("     (BAD) Data pointer is null\n");
    }
    if(err & VAR_DATA_BAD){
        printf_log("     (BAD) Data pointer is bad\n");
    }
    if(err & VAR_CANARY_L_BAD){
        printf_log("     (BAD) Left struct canary bad\n");
    }
    if(err & VAR_CANARY_R_BAD){
        printf_log("     (BAD) Right struct canary bad\n");
    }
    if(err & VAR_DATA_CANARY_L_BAD){
        printf_log("     (BAD) Left data canary bad\n");
    }
    if(err & VAR_DATA_CANARY_R_BAD){
        printf_log("     (BAD) Right data canary bad\n");
    }
    if(err & VAR_CANARY_L_BAD){
        printf_log("     (BAD) Left struct canary bad\n");
    }
    if(err & VAR_HASH_BAD){
        printf_log("     (BAD) Struct hash bad\n");
    }
    if(err & VAR_DATA_HASH_BAD){
        printf_log("     (BAD) Data hash bad\n");
    }
}

void printVarInfo_log(const VarInfo *var){
    setConsoleColor(stderr, COLOR_WHITE, COLOR_BLACK);

    if (var != nullptr){
    fprintf(_logfile, "     Variable info:     Name: %s\n"
                      "                      Status: %s\n"
                      "                  Created at: %s :%d\n"
                      "                 In function: %s\n",
    strPrintable(var->name), varstatusAsString(var->status) , strPrintable(var->file), var->line, strPrintable(var->func));

    fprintf( stderr , "     Variable info:     Name: %s\n"
                      "                      Status: %s\n"
                      "                  Created at: %s :%d\n"
                      "                 In function: %s\n",
    strPrintable(var->name), varstatusAsString(var->status) , strPrintable(var->file), var->line, strPrintable(var->func));
    }
    else{
        fprintf(_logfile, "     Variable info: info is nullptr\n");
        fprintf( stderr , "     Variable info: info is nullptr\n");
    }

    resetLogColor();

}

const char* varstatusAsString(variableStatus_t var){
    switch(var){
    case VARSTATUS_NEW:
        return "Uninitialised";
    case VARSTATUS_UNUSED:
        return "Initialised";
    case VARSTATUS_NORMAL:
        return "Normal";
    case VARSTATUS_DEAD:
        return "Dead";
    default:
        return "Invalid/uninitialised";
    }
}

const char* strPrintable(const char* str){
    if(str == nullptr){
        return "Nullptr";
    }
    return str;
}

bool checkLCanary(const void* ptr){
    return ((canary_t*)ptr)[-1]     == CANARY_L;
}

bool checkRCanary(const void* ptr, size_t len){
    return ((canary_t*)((char*)ptr+len))[0] == CANARY_R;
}

hash_t gnuHash(const void* begin_ptr, const void* end_ptr){
    hash_t hash = HASH_DEFAULT;
    while(begin_ptr < end_ptr){
        hash = (hash * 33) + (*(uint8_t*)begin_ptr);
        begin_ptr = (uint8_t*)begin_ptr + 1;
    }
    return hash;
}
