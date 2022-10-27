#ifndef DEBUG_UTILS_H_INCLUDED
#define DEBUG_UTILS_H_INCLUDED

#include <stdint.h>
#include "logging.h"

//#define NO_ERR_TRACE


#define BASE_ERRORS(_pref)    \
    _pref##_NOERROR = 0,      \
                              \
    _pref##_BADOP    = 1 << 0,\
    _pref##_INTERR   = 1 << 1,\
    _pref##_NULL     = 1 << 2,\
    _pref##_BAD      = 1 << 3,\
    _pref##_DEAD     = 1 << 4,\
    _pref##_CORRUPT  = 1 << 5,\
    _pref##_ERRUNK   = 1 << 6,


enum baseError_t {
    BASE_ERRORS(ERR)
};
void printBaseError_log(baseError_t err);

enum varError_t{
    BASE_ERRORS(VAR)

    VAR_DATA_NULL         = (1 << 8),
    VAR_DATA_BAD          = (1 << 9),
    VAR_BADSTATE          = (1 << 10),


    VAR_CANARY_L_BAD      = (1 << 11),
    VAR_CANARY_R_BAD      = (1 << 12),
    VAR_DATA_CANARY_L_BAD = (1 << 13),
    VAR_DATA_CANARY_R_BAD = (1 << 14),

    VAR_HASH_BAD          = (1 << 16),
    VAR_DATA_HASH_BAD     = (1 << 17)

};


#define passError(_err_t, _err )  {                   \
    if(_err != ERR_NOERROR){                        \
        int _ret_err = err & (ERR_INTERR | ERR_CORRUPT | ERR_ERRUNK); \
        if(_err & (ERR_NULL | ERR_BAD | ERR_DEAD)){     \
            _ret_err |= ERR_CORRUPT;                    \
        }                                               \
        if(_err & ERR_BADOP){                           \
            _ret_err |= ERR_ERRUNK;                     \
        }                                               \
        if(_ret_err == ERR_NOERROR){                    \
            _ret_err |= ERR_ERRUNK;                     \
        }                                               \
                                                        \
        /*#ifndef NO_ERR_TRACE                            \
        Error_log("Error %X passed from %X\n", _ret_err, _err);\
        #endif    */                                      \
        return (_err_t) _ret_err;                       \
    }                                                   \
}


#define retError(_err_t, _err )  {                        \
    if(_err != ERR_NOERROR){                            \
        /*#ifndef NO_ERR_TRACE                            \
        Error_log("Error %X returned", _ret_err, _err);  \
        #endif   */                                       \
        return (_err_t) _ret_err;                       \
    }                                                   \
}

enum variableStatus_t{
    VARSTATUS_NEW    = 0,
    VARSTATUS_UNUSED = 1,
    VARSTATUS_NORMAL = 2,
    VARSTATUS_DEAD   = 666
};

const char* varstatusAsString(variableStatus_t var);

struct VarInfo{
    const char* name;
    variableStatus_t status = VARSTATUS_NEW;
    const char* file;
    int line;
    const char* func;
};
void printVarInfo_log(const VarInfo *var);

const char* strPrintable(const char* str);

#ifdef varInfoInit
    #error redefinition of internal macro varInfoInit()
#endif
#define varInfoInit(var) {#var, VARSTATUS_UNUSED, __FILE__, __LINE__, __PRETTY_FUNCTION__}

//canary protection
typedef uint64_t canary_t;
const canary_t CANARY_L = 0xDEADBEEFDEADBEEF;
const canary_t CANARY_R = 0xFACEFEEDFACEFEED;

bool checkLCanary(const void* ptr);
bool checkRCanary(const void* ptr, size_t len);


//hash function
typedef uint64_t hash_t;
const hash_t HASH_DEFAULT = 0xDEFEC8EDBAADBEEF;

hash_t gnuHash(const void* begin_ptr, const void* end_ptr);

#endif // DEBUG_UTILS_H_INCLUDED
