#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "asserts.h"
#include "logging.h"
#include "debug_utils.h"

#include "Stack.h"



#ifndef STACK_NO_CANARY
    static const size_t STACK_DATA_BEGIN_OFFSET  =   sizeof(canary_t);
    static const size_t STACK_DATA_SIZE_OFFSET   = 2*sizeof(canary_t);
#else
    static const size_t STACK_DATA_BEGIN_OFFSET  = 0;
    static const size_t STACK_DATA_SIZE_OFFSET   = 0;
#endif

#ifndef STACK_NO_PROTECT
    #define stackCheckRet(__stk, ...)  \
        if(stackError(__stk)){             \
            Error_log("%s", "Stack error");\
            stackDump(__stk);              \
            return __VA_ARGS__;            \
        }
#else
    #define stackCheckRet(__stk, ...) ;
#endif

#ifndef STACK_NO_PROTECT
    #define stackCheckRetPtr(__stk, __errptr, ...)  \
        if(stackError(__stk)){                \
            Error_log("%s", "Stack error");   \
            stackDump(__stk);                 \
            if(__errptr)                      \
                *__errptr = stackError(__stk);\
            return __VA_ARGS__;               \
        }
#else
    #define stackCheckRetPtr(__stk, __errptr, ...)  ;
#endif

inline static void* stackDataMemBegin(const Stack* stk){
    assert_log(stk != nullptr);
    return ((uint8_t*)stk->data)-STACK_DATA_BEGIN_OFFSET;
}
inline static size_t stackDataMemSize(const Stack* stk){
    assert_log(stk != nullptr);
    return (stk->capacity*sizeof(ELEM_T)) + STACK_DATA_SIZE_OFFSET ;
}



#ifndef STACK_NO_HASH
    static hash_t stackGetDataHash(const Stack* stk){
        return gnuHash(stk->data, stk->data + stk->capacity);
    }
    static hash_t stackGetStructHash(const Stack* stk){
        return gnuHash(&(stk->data), &(stk->struct_hash));
    }

    stackError_t stackUpdHashes(Stack* stk){
        if (stk == nullptr)
            return STACK_NULL;
        if (IsBadWritePtr(stk, sizeof(stk)))
            return STACK_BAD;
        if (stk->data == nullptr && stk->capacity != 0)
            return STACK_DATA_NULL;
        if (stk->data == DESTRUCT_PTR)
            return STACK_DATA_NULL;
        if (stk->size > stk->capacity)
            return STACK_SIZE_CAP_BAD;

        stk->data_hash   = stackGetDataHash  (stk);
        stk->struct_hash = stackGetStructHash(stk);

        return STACK_NOERROR;
    }
#else
    stackError_t stackUpdHashes(Stack* stk){
        return STACK_NOERROR;
    }
#endif

bool stackCtor_(Stack* stk){
    #ifndef STACK_NO_PROTECT
    if (IsBadWritePtr(stk, sizeof(stk))){
        return false;
    }
    #endif
    stk->data = nullptr;
    stk->size = 0;
    stk->capacity = 0;

    #ifndef STACK_NO_CANARY
        stk->leftcan  = CANARY_L;
        stk->rightcan = CANARY_R;
    #endif
    return true;
}

stackError_t stackError(const Stack* stk){
    if (stk == nullptr)
        return STACK_NULL;

    if (IsBadReadPtr(stk, sizeof(stk)))
        return STACK_BAD;

    if (stk->size == SIZE_MAX || stk->capacity == SIZE_MAX || stk->data == DESTRUCT_PTR)
        return STACK_DEAD;


    unsigned int err = 0;

    if (stk->capacity != 0){
        if (stk->data == nullptr)
            err |= STACK_DATA_NULL;
        if (IsBadWritePtr(stackDataMemBegin(stk), stackDataMemSize(stk)))
            err |= STACK_DATA_BAD;
    }
    if (stk->size > stk->capacity)
        err |= STACK_SIZE_CAP_BAD;

    #ifndef STACK_NO_CANARY
        if (stk->leftcan != CANARY_L)
            err |= STACK_CANARY_L_BAD;
        if (stk->rightcan != CANARY_R)
            err |= STACK_CANARY_R_BAD;
    #endif

    #ifndef STACK_NO_HASH
        if (stk->struct_hash     != stackGetStructHash(stk))
            err |= STACK_HASH_BAD;
    #endif

    if ((err & (STACK_DATA_BAD | STACK_HASH_BAD)) || stk->data == nullptr){
        #ifndef STACK_NO_HASH
            if(stk->data_hash != HASH_DEFAULT)
                err |= STACK_DATA_HASH_BAD;
        #endif
        return (stackError_t)err;
    }

    #ifndef STACK_NO_CANARY
        if (!checkLCanary(stk->data))
            err |= STACK_DATA_CANARY_L_BAD;
        if (!checkRCanary(stk->data, stk->capacity * sizeof(ELEM_T)))
            err |= STACK_DATA_CANARY_R_BAD;
    #endif

    #ifndef STACK_NO_HASH
        if (stk->data_hash   != stackGetDataHash(stk))
            err |= STACK_DATA_HASH_BAD;
    #endif

    return (stackError_t)err;
}

static stackError_t stackError_dbg(Stack* stk){
    #ifndef STACK_NO_PROTECT
        return stackError(stk);
    #else
        return STACK_NOERROR;
    #endif
}


void stackDump(const Stack* stk){

    info_log("Stack dump:\n      stack at %p \n", stk);

    stackError_t err = stackError(stk);
    if (err & STACK_NULL){
        printf_log("      (BAD)  Stack poiner is null\n");
        return;
    }
    if (err & STACK_BAD){
        printf_log("      (BAD)  Stack poiner is invalid\n");
        return;
    }

    printf_log("      %ld/%ld elements\n", stk->size, stk->capacity);
    printf_log("      Data: %p\n", stk->data);

    if (err & STACK_DEAD){
        printf_log("      (BAD)  Stack was already destructed\n\n");
        return;
    }

    #ifndef STACK_NO_PROTECT
    printVarInfo_log(&(stk->info));
    #endif

    #ifndef STACK_NO_HASH
        if (err & STACK_HASH_BAD){
            printf_log("      (BAD)  Struct hash invalid. Written %p calculated %p\n", stk->struct_hash  , stackGetStructHash(stk));
        }
    #endif
    #ifndef STACK_NO_CANARY
        if (err & STACK_CANARY_L_BAD){
            printf_log("      (BAD)  Struct L canary BAD! Value: %p\n", stk->leftcan);
        }
        if (err & STACK_CANARY_R_BAD){
            printf_log("      (BAD)  Struct R canary BAD! Value: %p\n", stk->rightcan);
        }
    #endif

    if (stk->data == nullptr){
        printf_log("      (bad?) Stack data poiner is null\n\n");
        return;
    }

    if (err & STACK_DATA_BAD){
        printf_log("      (BAD)  Stack data poiner is invalid\n");
        dumpData(stackDataMemBegin(stk), stackDataMemSize(stk));
        return;
    }
    if (err & STACK_SIZE_CAP_BAD){
        printf_log("      (BAD)  Stack size is larger than capacity\n");
    }

    #ifndef STACK_NO_HASH
        hash_t data_hash = stackGetDataHash(stk);
        if (data_hash != stk->data_hash){
            printf_log("      (BAD)  Data hash invalid. Written %p calculated %p\n", stk->data_hash  , data_hash);
        }
    #endif
    #ifndef STACK_NO_CANARY
        if (!checkLCanary(stk->data)){
            printf_log("      (BAD)  Data L canary BAD! Value: %p\n", ((canary_t*)stk->data)[-1]);
        }
        if (!checkRCanary(stk->data, stk->capacity * sizeof(ELEM_T))){
            printf_log("      (BAD)  Data R canary BAD! Value: %p\n",*((canary_t*)stk->data + stk->capacity));
        }
    #endif
    printf_log("\n");

    for (size_t i = 0; i < stk->capacity; i++){
        printf_log("    ");

        printf_log("%c", (i < stk->size           ) ? '*':' ');

        printf_log("[%ld] " ELEM_SPEC " ", i, stk->data[i]);
        printf_log("%s", (stk->data[i] == BAD_ELEM) ? "(POISON)\n":"\n");
    }
    printf_log("\n");

}

stackError_t stackDtor(Stack* stk){
    stackCheckRet(stk, stackError_dbg(stk));

    for (size_t i = 0; i < stk->capacity; i++){
        stk->data[i] = BAD_ELEM;
    }
    if (stk->data != nullptr)
        free(stackDataMemBegin(stk));

    stk->data = DESTRUCT_PTR;
    stk->size = -1;
    stk->capacity = -1;
    #ifndef STACK_NO_PROTECT
    (stk->info).status = VARSTATUS_DEAD;
    #endif
    return STACK_NOERROR;
}



static stackError_t stackResize_(Stack* stk, size_t new_capacity){

    int err = stackError_dbg(stk);
    err &= ~(STACK_HASH_BAD | STACK_DATA_HASH_BAD);
    if (err)
        return (stackError_t)err;


    if (new_capacity < stk->size){
        return STACK_OP_INVALID;
    }

    errno = 0;
    ELEM_T* new_mem = nullptr;
    if (stk->data != nullptr){
        new_mem = (ELEM_T*)(
                            (char*)realloc(stackDataMemBegin(stk), new_capacity*sizeof(ELEM_T) + STACK_DATA_SIZE_OFFSET)
                            + STACK_DATA_BEGIN_OFFSET);
    }
    else{
        new_mem = (ELEM_T*)(
                            (char*)calloc(                         new_capacity*sizeof(ELEM_T) +  STACK_DATA_SIZE_OFFSET, 1)
                            + STACK_DATA_BEGIN_OFFSET);
    }
    if (new_mem == nullptr){
        perror_log("error while reallocating memory for stack");
        return STACK_OP_ERROR;
    }
    stk->data = new_mem;

    #ifndef STACK_NO_CANARY
        *((canary_t*)(stk->data + new_capacity)) = CANARY_R;
        *((canary_t*)(stk->data)-1)              = CANARY_L;
    #endif

    #ifndef STACK_NO_PROTECT
        for (size_t i = stk->capacity; i < new_capacity; i++){
            stk->data[i] = BAD_ELEM;
        }
    #endif
    stk->capacity = new_capacity;
    return STACK_NOERROR;
}

stackError_t stackResize(Stack* stk, size_t new_capacity){
    stackError_t err = stackResize_(stk, new_capacity);
    if(err == 0)
        stackUpdHashes(stk);
    return err;
}



stackError_t stackPush(Stack* stk, ELEM_T elem){
    stackCheckRet(stk, stackError_dbg(stk));
    #ifndef STACK_NO_PROTECT
    (stk->info).status = VARSTATUS_NORMAL;
    #endif

    if (stk->size == stk->capacity){
        stackError_t err = stackResize_(stk, (stk->capacity == 0)? STACK_MIN_SIZE : stk->capacity*2);
        if (err != STACK_NOERROR)
            return err;
    }

    stk->data[stk->size++] = elem;
    stackUpdHashes(stk);

    return stackError_dbg(stk);
}

ELEM_T stackTop(Stack* stk, stackError_t *err_ptr){
    stackCheckRetPtr(stk, err_ptr, BAD_ELEM);

    if (stk->size == 0){
        if (err_ptr)
            *err_ptr = STACK_OP_INVALID;
        return BAD_ELEM;
    }
    return stk->data[stk->size-1];
}

ELEM_T stackGet(Stack* stk, size_t addr, stackError_t *err_ptr){
    stackCheckRetPtr(stk, err_ptr, BAD_ELEM);

    if (stk->size <= addr){
        if (err_ptr)
            *err_ptr = STACK_OP_INVALID;
        return BAD_ELEM;
    }
    return stk->data[stk->size-addr-1];
}

ELEM_T stackPop(Stack* stk, stackError_t *err_ptr){
    stackCheckRetPtr(stk, err_ptr, BAD_ELEM);

    if (stk->size == 0){
        if (err_ptr)
            *err_ptr = STACK_OP_INVALID;
        return BAD_ELEM;
    }

    ELEM_T ret = stk->data[--stk->size];

    #ifndef STACK_NO_PROTECT
        stk->data[stk->size] = BAD_ELEM;
    #endif

    if (stk->size * 2 < stk->capacity && stk->capacity > 2*STACK_MIN_SIZE){
        stackError_t err = stackResize_(stk, (stk->capacity == 0)? STACK_MIN_SIZE : stk->size*2);
        if (err != STACK_NOERROR){
            if (err_ptr)
                *err_ptr = err;
            return BAD_ELEM;
        }

    }

    stackUpdHashes(stk);
    return ret;
}
