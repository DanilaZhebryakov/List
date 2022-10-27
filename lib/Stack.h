#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#include "logging.h"
#include "debug_utils.h"

#define ELEM_T int

#define ELEM_SPEC "%d"
#define BAD_ELEM 133
#define DESTRUCT_PTR ((ELEM_T*)0xBAD)
#define STACK_MIN_SIZE 10

enum stackError_t{
    STACK_NOERROR           = 0,

    STACK_NULL              = 1 << 0,
    STACK_BAD               = 1 << 1,
    STACK_DEAD              = 1 << 2,
    STACK_DATA_NULL         = 1 << 3,
    STACK_DATA_BAD          = 1 << 4,
    STACK_SIZE_CAP_BAD      = 1 << 5,

    STACK_CANARY_L_BAD      = 1 << 8,
    STACK_CANARY_R_BAD      = 1 << 9,
    STACK_DATA_CANARY_L_BAD = 1 << 10,
    STACK_DATA_CANARY_R_BAD = 1 << 11,

    STACK_HASH_BAD          = 1 << 16,
    STACK_DATA_HASH_BAD     = 1 << 17,

    STACK_OP_INVALID        = 1 << 24,
    STACK_OP_ERROR          = 1 << 25
};

#ifdef NDEBUG
    #define STACK_NO_PROTECT
#endif

#ifdef STACK_NO_PROTECT
    #define STACK_NO_HASH
    #define STACK_NO_CANARY
#endif



struct Stack{
    #ifndef STACK_NO_CANARY
        canary_t leftcan;
    #endif

    ELEM_T *data;
    size_t size;
    size_t capacity;

    #ifndef STACK_NO_PROTECT
        VarInfo info;
    #endif
    #ifndef STACK_NO_HASH
        hash_t data_hash;
        hash_t struct_hash;
    #endif
    #ifndef STACK_NO_CANARY
        canary_t rightcan;
    #endif
};

stackError_t stackUpdHashes(Stack* stk);

bool stackCtor_(Stack* stk);

stackError_t stackError(const Stack* stk);

void stackDump(const Stack* stk);

stackError_t stackDtor(Stack* stk);

stackError_t stackResize(Stack* stk, size_t new_capacity);

stackError_t stackPush(Stack* stk, ELEM_T elem);

ELEM_T stackTop(Stack* stk, stackError_t *err_ptr = nullptr);

ELEM_T stackGet(Stack* stk, size_t addr, stackError_t *err_ptr = nullptr);

ELEM_T stackPop(Stack* stk, stackError_t *err_ptr = nullptr);


#ifdef stackCtor
    #error redefinition of internal macro stackCtor
#endif
#ifndef STACK_NO_PROTECT
    #define stackCtor(__stk)    \
        if (stackCtor_(__stk)){  \
            (__stk)->info = varInfoInit(__stk); \
            stackUpdHashes(__stk);  \
        }                       \
        else {                  \
            Error_log("%s", "bad ptr passed to constructor\n");\
        }
#else
    #define stackCtor(__stk)    \
            stackCtor_(__stk);
#endif

#endif // STACK_H_INCLUDED
