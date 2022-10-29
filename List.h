#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

#include "lib/debug_utils.h"
#include "lib/logging.h"

#define LIST_ELEM_T int
#define LIST_BADELEM 404
#define LIST_ELEM_SPEC "d"
//#define LIST_NOPROTECT
//#define LIST_NOCANARY


struct List{
    #ifndef LIST_NOCANARY
        canary_t leftcan;
    #endif
    #ifndef LIST_NOPROTECT
        VarInfo info;
    #endif
    size_t capacity;
    size_t size;

    LIST_ELEM_T* data;
    size_t*      next;
    size_t*      prev;

    size_t fmem_stack;
    size_t fmem_end;

    bool sorted = true;
    #ifndef LIST_NOCANARY
        canary_t rightcan;
    #endif
};

#ifdef listCtor
    #error redefinition of internal macro stackCtor
#endif
#ifndef LIST_NO_PROTECT
    #define listCtor(_lst)      \
        if (listCtor_(_lst)){  \
            (_lst)->info = varInfoInit(_lst); \
        }                       \
        else {                  \
            Error_log("%s", "bad ptr passed to constructor\n");\
        }
#else
    #define listCtor(__stk)    \
            listCtor_(__stk);
#endif

bool listCtor_(List* lst);

varError_t listError(const List* lst);

void listDump(const List* lst, bool graph_dump = true);

varError_t listDtor(List* lst);

varError_t listResize(List* lst, size_t new_capacity);

size_t listPushAfter(List* lst, size_t ind, LIST_ELEM_T elem, varError_t* err_ptr);

varError_t listDeleteElem(List* lst, size_t ind);

varError_t listSerialize(List* lst, size_t new_size);

#endif // LIST_H_INCLUDED
