
#include "List.h"
#include "lib/System_utils.h"

#ifndef LIST_NO_CANARY
    static const ptrdiff_t LIST_DATA_BEGIN_OFFSET  =   sizeof(canary_t)-sizeof(LIST_ELEM_T);
    static const ptrdiff_t LIST_DATA_PTR_BEGIN_OFFSET  =   sizeof(canary_t)-sizeof(size_t);
    static const ptrdiff_t LIST_DATA_SIZE_OFFSET   = 2*sizeof(canary_t);

#else
    static const ptrdiff_t LIST_DATA_BEGIN_OFFSET  = -sizeof(LIST_ELEM_T);
    static const ptrdiff_t LIST_DATA_SIZE_OFFSET   = 0;
#endif

#define DESTRUCT_PTR ((void*)0xBAD)

#ifndef LIST_NO_PROTECT
    #define listCheckRet(__lst, ...)  \
        if(listError(__lst)){             \
            Error_log("%s", "List error");\
            listDump(__lst);              \
            return __VA_ARGS__;            \
        }
#else
    #define listCheckRet(__lst, ...) ;
#endif

#ifndef LIST_NO_PROTECT
    #define listCheckRetPtr(__lst, __errptr, ...)  \
        if(listError(__lst)){                \
            Error_log("%s", "List error");   \
            listDump(__lst);                 \
            if(__errptr)                      \
                *__errptr = listError(__lst);\
            return __VA_ARGS__;               \
        }
#else
    #define listCheckRetPtr(__lst, __errptr, ...)  ;
#endif

inline static void* listDataMemBegin(const List* lst){
    assert_log(lst != nullptr);
    return ((uint8_t*)lst->data)-LIST_DATA_BEGIN_OFFSET;
}
inline static size_t listDataMemSize(const List* lst){
    assert_log(lst != nullptr);
    return (lst->capacity*sizeof(LIST_ELEM_T)) + LIST_DATA_SIZE_OFFSET ;
}

inline static size_t listDataPtrMemSize(const List* lst){
    assert_log(lst != nullptr);
    return (lst->capacity*sizeof(size_t)) + LIST_DATA_SIZE_OFFSET;
}

bool listCtor_(List* lst){
    #ifndef LIST_NO_PROTECT
    if (!isPtrWritable(lst, sizeof(lst))){
        return false;
    }
    #endif

    lst->data = nullptr;
    lst->prev = nullptr;
    lst->next = nullptr;
    lst->head = 0;
    lst->tail = 0;

    lst->fmem_stack = 0;
    lst->fmem_end   = 1;

    lst->capacity = 0;
    lst->size = 0;

    #ifndef LIST_NO_CANARY
        lst->leftcan  = CANARY_L;
        lst->rightcan = CANARY_R;
    #endif
    return true;
}

varError_t listError(const List* lst){

    if (lst == nullptr)
        return VAR_NULL;

    if (!isPtrReadable(lst, sizeof(lst)))
        return VAR_BAD;

    if (lst->head == SIZE_MAX || lst->tail == SIZE_MAX  || lst->capacity == SIZE_MAX || lst->data == DESTRUCT_PTR)
        return VAR_DEAD;


    unsigned int err = 0;

    if (lst->capacity != 0){
        if (lst->data == nullptr)
            err |= VAR_DATA_NULL;
        if (!isPtrWritable(listDataMemBegin(lst), listDataMemSize(lst)))
            err |= VAR_DATA_BAD;

        if (lst->prev == nullptr)
            err |= VAR_DATA_NULL;
        if (!isPtrWritable(lst->prev - LIST_DATA_BEGIN_OFFSET, listDataPtrMemSize(lst)))
            err |= VAR_DATA_BAD;

        if (lst->next == nullptr)
            err |= VAR_DATA_NULL;
        if (!isPtrWritable(lst->next - LIST_DATA_BEGIN_OFFSET, listDataPtrMemSize(lst)))
            err |= VAR_DATA_BAD;
    }

    if (lst->head       >= lst->fmem_end ||
        lst->tail       >= lst->fmem_end ||
        lst->fmem_stack >= lst->fmem_end ||
        lst->size       >= lst->fmem_end ||
        lst->fmem_end   > lst->capacity + 1)
    {
        err |= VAR_BADSTATE;
    }

    #ifndef LIST_NO_CANARY
        if (lst->leftcan != CANARY_L){
            err |= VAR_CANARY_L_BAD;
        }
        if (lst->rightcan != CANARY_R){
            err |= VAR_CANARY_R_BAD;
        }
    #endif

    if ((err & VAR_DATA_BAD) || lst->data == nullptr){
        return (varError_t)err;
    }

    #ifndef LIST_NO_CANARY
        if (!checkLCanary(lst->data + 1))
            err |= VAR_DATA_CANARY_L_BAD;
        if (!checkRCanary(lst->data, (lst->capacity + 1) * sizeof(LIST_ELEM_T)))
            err |= VAR_DATA_CANARY_R_BAD;

        if (!checkLCanary(lst->prev + 1))
            err |= VAR_DATA_CANARY_L_BAD;
        if (!checkRCanary(lst->prev, (lst->capacity + 1) * sizeof(size_t)))
            err |= VAR_DATA_CANARY_R_BAD;

        if (!checkLCanary(lst->next + 1))
            err |= VAR_DATA_CANARY_L_BAD;
        if (!checkRCanary(lst->next, (lst->capacity + 1) * sizeof(size_t)))
            err |= VAR_DATA_CANARY_R_BAD;
    #endif

    if(err != 0){
        err |= VAR_CORRUPT;
    }

    return (varError_t)err;
}

static varError_t listError_dbg(List* lst){
    #ifndef LIST_NO_PROTECT
        return listError(lst);
    #else
        return VAR_NOERROR;
    #endif
}

void listDump(const List* lst, bool graph_dump){

    varError_t err = listError(lst);
    printf_log("List dump\n");
    printf_log("    List at %p\n", lst);

    bool ptr_ok = !(err & VAR_NULL || err & VAR_BAD);

    if(ptr_ok){
        printf_log("    Head: %lu Tail %lu Capacity %lu Size %lu\n", lst->head, lst->tail, lst->capacity, lst->size);
        printf_log("    Free mem ptr: Stack: %lu Unused end: %lu", lst->fmem_stack, lst->fmem_end);
    }

    if(err == VAR_NOERROR){
       printf_log("    List ok\n", lst);
    }
    else{
       printf_log("    ERRORS:\n", lst);
    }

    printBaseError_log((baseError_t) err);
    if(!ptr_ok){
        return;
    }

    if(err & VAR_BADSTATE){
        printf_log("     (BAD) List in invalid state (indexes out of range)\n");
    }

    #ifndef LIST_NOCANARY
        if(err & VAR_CANARY_L_BAD){
            printf_log("     Left struct canary bad (%X | %X)\n", lst->leftcan , CANARY_L);
        }
        if(err & VAR_CANARY_R_BAD){
            printf_log("     Right struct canary bad (%X | %X)\n", lst->rightcan , CANARY_R);
        }
    #endif
    if(err & VAR_DATA_NULL){
        printf_log("     Data pointer is null\n");
        return;
    }
    if(err & VAR_DATA_BAD){
        printf_log("     Data pointer is bad\n");
        return;
    }

    if (lst->data == nullptr){
        printf_log("     List empty\n");
        return;
    }
    #ifndef LIST_NOCANARY
        if(err & VAR_DATA_CANARY_L_BAD){
            printf_log("     Left data canary bad (D%X-P%X-N%X | %X)\n",
                        (canary_t*)(lst->data+1)[-1], (canary_t*)(lst->data+1)[-1], (canary_t*)(lst->data+1)[-1], CANARY_L);
        }
        if(err & VAR_DATA_CANARY_R_BAD){
            size_t cap = lst->capacity;
            printf_log("     Right data canary bad (D%X-P%X-N%X | %X)\n",
                       (canary_t*)(lst->data+1)[-1], (canary_t*)(lst->data+1)[-1], (canary_t*)(lst->data+1)[-1], CANARY_R);
        }
    #endif

    printf_log("I |");
    for(int i = 1; i <= lst->capacity; i++){
        printf_log("%5d", i);
    }
    printf_log("\nD |");
    for(int i = 1; i <= lst->capacity; i++){
        printf_log("%5" LIST_ELEM_SPEC, lst->data[i]);
    }
    printf_log("\nP |");
    for(int i = 1; i <= lst->capacity; i++){
        printf_log("%5lu", lst->prev[i]);
    }
    printf_log("\nN |");
    for(int i = 1; i <= lst->capacity; i++){
        printf_log("%5lu", lst->next[i]);
    }
    printf_log("\n");

    printf_log("list elements in order:\n");
    if (err == VAR_NOERROR){
        size_t i = lst->head;
        while(i != 0){
            printf_log("%d ", lst->data[i]);
            i = lst->next[i];
        }
        printf_log("\n");
    }
    if (graph_dump && lst->capacity != 0){
        FILE* graph_file = fopen("graph.tmp", "w");
        fprintf(graph_file, "digraph G{\n");
        fprintf(graph_file, " rankdir=LR;\n"
                            "node[shape=rectangle, style=filled, fillcolor=lightgrey]");

        for(int i = 1; i <= lst->capacity; i++){
            fprintf(graph_file, "\"N%d\"[label=\"[%d] %d\"", i, i, lst->data[i]);
            if(lst->prev[i] == i){
                fprintf(graph_file, ",color=red");
            }
            if(i >= lst->fmem_end){
                fprintf(graph_file, ",color=orange");
            }

            fprintf(graph_file, "]\n");
        }
        for(int i = 1; i < lst->capacity; i++){
            fprintf(graph_file, "\"N%d\"->", i, i, lst->data[i]);
        }
        fprintf(graph_file, "\"N%d\"[style=invis, weight=100]\n", lst->capacity, lst->capacity, lst->data[lst->capacity]);
        size_t i = lst->head;
        while(lst->next[i] != 0){
            fprintf(graph_file, "N%d->N%d\n", i, lst->next[i]);
            i = lst->next[i];
        }
        fprintf(graph_file, "HEAD[shape=ellipse, color=grey]\n");
        fprintf(graph_file, "HEAD->N%d\n", lst->head);
        fprintf(graph_file, "TAIL[shape=ellipse, color=grey]\n");
        fprintf(graph_file, "TAIL->N%d\n", lst->tail);

        fprintf(graph_file, "}");
        fclose(graph_file);
        system("dot -Tpng graph.tmp -ograph.png");
    }
}

varError_t listDtor(List* lst){
    listCheckRet(lst, listError_dbg(lst));

    for (size_t i = 0; i < lst->capacity; i++){
        lst->data[i] = LIST_BADELEM;
    }
    if (lst->data != nullptr)
        free(listDataMemBegin(lst));

    lst->data = (LIST_ELEM_T*)DESTRUCT_PTR;
    lst->prev = (size_t*)DESTRUCT_PTR;
    lst->next = (size_t*)DESTRUCT_PTR;
    lst->capacity = -1;
    lst->head = -1;
    lst->tail = -1;
    #ifndef LIST_NO_PROTECT
    (lst->info).status = VARSTATUS_DEAD;
    #endif
    return VAR_NOERROR;
}

static bool allocOrResize(void** ptr, size_t new_size , size_t offset){
    void* new_mem = nullptr;
    errno = 0;

    if (*ptr != nullptr){
        new_mem = (void*)(
                            (char*)realloc((char*)(*ptr) - offset, new_size)
                            + offset);
    }
    else{
        new_mem = (LIST_ELEM_T*)(
                            (char*)calloc(                         new_size, 1)
                            + offset);
    }
    if (new_mem == nullptr){
        perror_log("error while reallocating memory");
        return false;
    }
    *ptr = new_mem;
    return true;
}

static void listReplaceDataCanary(List* lst){
        #ifndef LIST_NO_CANARY
            *((canary_t*)(lst->data + lst->capacity + 1)) = CANARY_R;
            *((canary_t*)(lst->prev + lst->capacity + 1)) = CANARY_R;
            *((canary_t*)(lst->next + lst->capacity + 1)) = CANARY_R;

            *(canary_t*)((char*)(lst->data) - LIST_DATA_BEGIN_OFFSET    ) = CANARY_L;
            *(canary_t*)((char*)(lst->prev) - LIST_DATA_PTR_BEGIN_OFFSET) = CANARY_L;
            *(canary_t*)((char*)(lst->next) - LIST_DATA_PTR_BEGIN_OFFSET) = CANARY_L;
        #endif
}

varError_t listResize(List* lst, size_t new_capacity){
    listCheckRet(lst, listError_dbg(lst));

    if(new_capacity < lst->fmem_end - 1){
        return VAR_BADOP;
    }

    if(allocOrResize((void**)&(lst->data), new_capacity*sizeof(LIST_ELEM_T) + LIST_DATA_SIZE_OFFSET , LIST_DATA_BEGIN_OFFSET) &&
       allocOrResize((void**)&(lst->prev), new_capacity*sizeof(size_t)      + LIST_DATA_SIZE_OFFSET , LIST_DATA_PTR_BEGIN_OFFSET) &&
       allocOrResize((void**)&(lst->next), new_capacity*sizeof(size_t)      + LIST_DATA_SIZE_OFFSET , LIST_DATA_PTR_BEGIN_OFFSET)
       ) {

        size_t t = lst->capacity;
        lst->capacity = new_capacity;


        #ifndef LIST_NO_PROTECT
        for(size_t i = t + 1; i <= new_capacity; i++){
            lst->data[i] = LIST_BADELEM;
            lst->prev[i] = 0;
            lst->next[i] = 0;
        }
        #endif

        listReplaceDataCanary(lst);
        return VAR_NOERROR;
    }
    else{
        Error_log("%s", "error while resizing list\n");
        return VAR_INTERR;
    }
}

static void listAddFreeMem(List* lst, size_t ind){
    lst->next[ind] = lst->fmem_stack;
    lst->prev[ind] = ind;
    lst->fmem_stack = ind;
    return;
}

static size_t listGetFreeMem(List* lst){
    if(lst->fmem_stack == 0){
        if(lst->fmem_end <= lst->capacity)
            return lst->fmem_end++;
        else
            return 0;
    }
    else{
        size_t t = lst->fmem_stack;
        lst->fmem_stack = lst->next[t];
        return t;
    }
}

size_t listPushAfter(List* lst, size_t ind, LIST_ELEM_T elem, varError_t* err_ptr){
    listCheckRetPtr(lst, err_ptr, 0);
    varError_t err = VAR_NOERROR;

    size_t ni = listGetFreeMem(lst);

    if(ni == 0){
        int new_cap = lst->capacity * 2;
        if(new_cap < 10){
            new_cap = 10;
        }

        err = listResize(lst, new_cap);
        if(err != VAR_NOERROR)
            passError(varError_t, err);

        ni = listGetFreeMem(lst);
        if(ni == 0){
            if(err_ptr){
                *err_ptr = VAR_ERRUNK;
                return 0;
            }
        }
    }
    lst->size++;

    lst->data[ni] = elem;
    lst->prev[ni] = ind;

    if(ind == 0){
        if(lst->head != 0){
            lst->next[ni] = lst->head;
            lst->prev[lst->head] = ni;
        }
        lst->head = ni;
    }
    else{
        lst->next[ni] = lst->next[ind];
        if(lst->next[ind] != 0){
            lst->prev[lst->next[ind]] = ni;
        }
        lst->next[ind] = ni;
    }

    if(ind == lst->tail){
        lst->tail = ni;
    }

    return ni;
}

varError_t listDeleteElem(List* lst, size_t ind){
    listCheckRet(lst, listError_dbg(lst));
    if(ind == 0 || ind >= lst->fmem_end || lst->prev[ind] == ind){
        return VAR_BADOP;
    }

    lst->size--;
    if(lst->prev[ind] != 0)
        lst->next[lst->prev[ind]] = lst->next[ind];
    else
        lst->head = lst->next[ind];

    if(lst->next[ind] != 0)
        lst->prev[lst->next[ind]] = lst->prev[ind];
    else
        lst->tail = lst->prev[ind];

    listAddFreeMem(lst, ind);
    return VAR_NOERROR;
}

varError_t listSerialize(List* lst, size_t new_size){
    listCheckRet(lst, listError_dbg(lst));
    LIST_ELEM_T* new_data = nullptr;
    size_t* new_prev = nullptr;
    size_t* new_next = nullptr;

    if(new_size < lst->size){
        return VAR_BADOP;
    }

    if(allocOrResize((void**)&(new_data), new_size*sizeof(LIST_ELEM_T) + LIST_DATA_SIZE_OFFSET , LIST_DATA_BEGIN_OFFSET)) {
        int ni = 1;
        int oi = lst->head;
        while(ni <= lst->size && oi != 0){
            new_data[ni] = lst->data[oi];
            oi = lst->next[oi];
            ni++;
        }
        if(ni <= lst->size || oi != 0){
            free(new_data);
            return VAR_CORRUPT;
        }

        if( allocOrResize((void**)(&new_prev), new_size*sizeof(size_t)      + LIST_DATA_SIZE_OFFSET , LIST_DATA_PTR_BEGIN_OFFSET) &&
            allocOrResize((void**)(&new_next), new_size*sizeof(size_t)      + LIST_DATA_SIZE_OFFSET , LIST_DATA_PTR_BEGIN_OFFSET)   ) {

            free(((char*)lst->data) - LIST_DATA_BEGIN_OFFSET);
            free(((char*)lst->prev) - LIST_DATA_PTR_BEGIN_OFFSET);
            free(((char*)lst->next) - LIST_DATA_PTR_BEGIN_OFFSET);

            lst->data = new_data;
            lst->prev = new_prev;
            lst->next = new_next;
            lst->capacity = new_size;
            lst->head = 1;
            lst->tail = lst->size;

            lst->fmem_end = lst->size + 1;
            lst->fmem_stack = 0;

            for(int i = 1; i <= lst->size; i++){
                new_prev[i] = i-1;
                new_next[i] = i+1;
            }
            new_next[lst->size] = 0;

            listReplaceDataCanary(lst);
            return VAR_NOERROR;
        }
    }
    if(new_prev != nullptr)
        free(new_prev);
    if(new_next != nullptr)
        free(new_next);
    if(new_data != nullptr)
        free(new_data);
    return VAR_INTERR;
}

