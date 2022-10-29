
#include "List.h"
#include "lib/System_utils.h"

#ifndef LIST_NO_CANARY
    static const ptrdiff_t LIST_DATA_BEGIN_OFFSET      =   sizeof(canary_t);
    static const ptrdiff_t LIST_DATA_PTR_BEGIN_OFFSET  =   sizeof(canary_t);
    static const ptrdiff_t LIST_DATA_SIZE_OFFSET       = 2*sizeof(canary_t);
    static const ptrdiff_t LIST_DATA_PTR_SIZE_OFFSET   = 2*sizeof(canary_t);

#else
    static const ptrdiff_t LIST_DATA_BEGIN_OFFSET      = 0;
    static const ptrdiff_t LIST_DATA_PTR_BEGIN_OFFSET  = 0;
    static const ptrdiff_t LIST_DATA_SIZE_OFFSET       = 0;
    static const ptrdiff_t LIST_DATA_PTR_SIZE_OFFSET   = 0;
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

    if (lst->capacity == SIZE_MAX || lst->data == DESTRUCT_PTR)
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

    if (lst->fmem_stack >= lst->fmem_end ||
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
    if (lst->next[0]    >= lst->fmem_end ||
        lst->prev[0]    >= lst->fmem_end){
        err |= VAR_BADSTATE;
    }


    #ifndef LIST_NO_CANARY
        if (!checkLCanary(lst->data))
            err |= VAR_DATA_CANARY_L_BAD;
        if (!checkRCanary(lst->data, (lst->capacity + 1) * sizeof(LIST_ELEM_T)))
            err |= VAR_DATA_CANARY_R_BAD;

        if (!checkLCanary(lst->prev))
            err |= VAR_DATA_CANARY_L_BAD;
        if (!checkRCanary(lst->prev, (lst->capacity + 1) * sizeof(size_t)))
            err |= VAR_DATA_CANARY_R_BAD;

        if (!checkLCanary(lst->next))
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

static void listTextDump(const List* lst){
    printf_log("I |");
    for (size_t i = 1; i <= lst->capacity; i++){
        printf_log("%5d", i);
    }
    printf_log("\nD |");
    for (size_t i = 1; i <= lst->capacity; i++){
        printf_log("%5" LIST_ELEM_SPEC, lst->data[i]);
    }
    printf_log("\nP |");
    for (size_t i = 1; i <= lst->capacity; i++){
        printf_log("%5lu", lst->prev[i]);
    }
    printf_log("\nN |");
    for (size_t i = 1; i <= lst->capacity; i++){
        printf_log("%5lu", lst->next[i]);
    }
    printf_log("\n");

    printf_log("list elements in order:\n");
    size_t i = lst->next[0];
    while (i != 0 && i < lst->capacity){
        printf_log("%d ", lst->data[i]);
        i = lst->next[i];
    }
    if (i != 0){
        printf_log("...Bad pointer");
    }
    printf_log("\n");
}

static void listGraphDump(const List* lst){
    #define COLOR_NORM_LINE    "\"#f0f0f0\""
    #define COLOR_NORM_TXT     "\"#f0f0f0\""
    #define COLOR_NORM_FILL    "\"#25252F\""
    #define COLOR_VALID_LINE   "\"#dad0ac\""
    #define COLOR_INVALID_LINE "\"#ff3e3e\""

    #define COLOR_FREE_S_LINE  "\"#00b4ed\""
    #define COLOR_FREE_S_FILL  "\"#00384a\""
    #define COLOR_FREE_E_LINE  "\"#4aec6d\""
    #define COLOR_FREE_E_FILL  "\"#053500\""


    FILE* graph_file = fopen("graph.tmp", "w");
    fprintf(graph_file, "digraph G{\n");
    fprintf(graph_file, "rankdir=LR; bgcolor=\"#151515\";\n"
                        "node[shape=rectangle, style=filled, fillcolor=" COLOR_NORM_FILL ", color=" COLOR_NORM_LINE ", fontcolor=" COLOR_NORM_TXT "]\n"
                        "edge[weight=1, color=\"#f0f0f0\"]\n");

    fprintf(graph_file, "\"N0\"[shape=diamond, label=\"[0]\", color=\"#6e00ff\"]\n");

    //draw main nodes
    for (size_t i = 1; i <= lst->capacity; i++){
        const char* bgcolor   = COLOR_NORM_FILL;
        const char* linecolor = COLOR_NORM_LINE;
        if (lst->prev[i] == i){
            linecolor = COLOR_FREE_S_LINE;
            bgcolor   = COLOR_FREE_S_FILL;
        }
        if (i >= lst->fmem_end){
            linecolor = COLOR_FREE_E_LINE;
            bgcolor   = COLOR_FREE_E_FILL;
        }

        fprintf(graph_file, "\"N%d\"[shape=plaintext, style=solid, color = %s, "
                "label=<<TABLE  BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" BGCOLOR = %s>\n"
                "<TR><TD>D: %" LIST_ELEM_SPEC "</TD></TR>\n"
                "<TR><TD>P: %lu </TD></TR>\n"
                "<TR><TD>N: %lu </TD></TR>\n"
                "</TABLE>> ]\n"
                , i, linecolor, bgcolor, lst->data[i], lst->prev[i], lst->next[i]);
    }
    for (size_t i = 0; i < lst->capacity; i++){
        fprintf(graph_file, "\"N%d\"->", i);
    }
    fprintf(graph_file, "\"N%d\"[style=dotted, dir=none, weight=1000]\n", lst->capacity);

    // draw index nodes
    size_t drawn_size = lst->capacity;
    if (lst->fmem_end == drawn_size + 1)
        drawn_size = lst->fmem_end;

    for (size_t i = 0; i <= drawn_size; i++){
        fprintf(graph_file, "\"I%d\"[shape=plaintext, style=solid, label = \"[%d]\"]\n", i, i);
    }
    for (size_t i = 0; i < drawn_size; i++){
        fprintf(graph_file, "\"I%d\"->", i, i, lst->data[i]);
    }
    fprintf(graph_file, "\"I%d\"[style=dotted, dir=none, weight=1000]\n", drawn_size);


    for (int i = 0; i <= lst->capacity; i++){
        fprintf(graph_file, "{rank=same; \"I%d\"; \"N%d\"}", i, i);
    }

    //draw main edges
    for(size_t i = 0; i < lst->fmem_end; i++){
        fprintf(graph_file, "N%d->N%d[", i, lst->next[i]);

        if (lst->prev[i] == i){
            fprintf(graph_file, "color=" COLOR_FREE_S_LINE ", style=dashed");
        }
        else{
            if(lst->prev[lst->next[i]] == i){
                fprintf(graph_file, "dir=both, arrowtail=crow, color=" COLOR_VALID_LINE );
            }
            else{
                fprintf(graph_file, "color=" COLOR_INVALID_LINE);
            }
        }
        fprintf(graph_file, "]\n");

        if (lst->next[lst->prev[i]] != i){
            fprintf(graph_file, "N%d->N%d[arrowhead=crow, constraint=false", i, lst->prev[i]);
            if (lst->prev[i] == i){
                fprintf(graph_file, ",color=" COLOR_FREE_S_LINE ", style=dashed");
            }
            else{
                fprintf(graph_file, ",color=" COLOR_INVALID_LINE);
            }
            fprintf(graph_file, "]\n");
        }
    }

    // draw pointer nodes
    fprintf(graph_file, "HEAD[shape=ellipse, color=grey]\n");
    fprintf(graph_file, "HEAD->N%d\n"                    , lst->next[0]);
    fprintf(graph_file, "I%d->HEAD[style=invis]\n"       , lst->next[0]);
    fprintf(graph_file, "{rank=same; \"N%d\"; \"HEAD\" }", lst->next[0]);

    fprintf(graph_file, "TAIL[shape=ellipse, color=grey]\n");
    fprintf(graph_file, "TAIL->N%d[arrowhead=crow]\n"    , lst->prev[0]);
    fprintf(graph_file, "I%d->TAIL[style=invis]\n"       , lst->prev[0]);
    fprintf(graph_file, "{rank=same; \"N%d\"; \"TAIL\" }", lst->prev[0]);

    fprintf(graph_file, "FREE_STK[shape=ellipse, color=" COLOR_FREE_S_LINE "]\n");
    fprintf(graph_file, "FREE_STK->N%d[color=" COLOR_FREE_S_LINE ", style=dashed]\n", lst->fmem_stack);
    fprintf(graph_file, "I%d->FREE_STK[style=invis]\n"                              , lst->fmem_stack);
    fprintf(graph_file, "{rank=same; \"N%d\"; \"FREE_STK\" }", lst->fmem_stack);

    if (lst->fmem_end <= drawn_size){
        fprintf(graph_file, "FREE_END[shape=ellipse, color=" COLOR_FREE_E_LINE "]\n");
        fprintf(graph_file, "I%d->FREE_END[style=invis]\n"                              , lst->fmem_end);
        fprintf(graph_file, "{rank=same; \"I%d\"; \"FREE_END\" }"                       , lst->fmem_end);
    }
    if (lst->fmem_end <= lst->capacity){
        fprintf(graph_file, "FREE_END->N%d[color=" COLOR_FREE_E_LINE ", style=dashed]\n", lst->fmem_end);
    }

    fprintf(graph_file, "}");
    fclose(graph_file);

    char cmd_str[100] = "dot -Tpng graph.tmp -o";
    embedNewDumpFile(cmd_str + strlen(cmd_str), "List_dump", ".png", "img");

    system(cmd_str);
}

void listDump(const List* lst, bool graph_dump){
    hline_log();
    varError_t err = listError(lst);
    printf_log("List dump\n");
    printf_log("    List at %p\n", lst);

    bool ptr_ok = !(err & VAR_NULL || err & VAR_BAD);

    printf_log("    Data: %p\n", lst->data);
    printf_log("    Prev: %p\n", lst->prev);
    printf_log("    Next: %p\n", lst->next);
    printf_log("    Sort: %s\n", lst->sorted ? "true" : "false");

    if (err == VAR_NOERROR){
       printf_log("    List ok\n", lst);
    }
    else {
       printf_log("    ERRORS:\n", lst);
    }

    printBaseError_log((baseError_t) err);
    if (!ptr_ok){
        hline_log();
        return;
    }

    #ifndef LIST_NOCANARY
        if (err & VAR_CANARY_L_BAD){
            printf_log("     Left struct canary bad (%X | %X)\n", lst->leftcan , CANARY_L);
        }
        if (err & VAR_CANARY_R_BAD){
            printf_log("     Right struct canary bad (%X | %X)\n", lst->rightcan , CANARY_R);
        }
    #endif
    if (err & VAR_DATA_NULL){
        printf_log("     Data pointer is null\n");
        hline_log();
        return;
    }
    if (err & VAR_DATA_BAD){
        printf_log("     Data pointer is bad\n");
        hline_log();
        return;
    }

    if (lst->data == nullptr){
        printf_log("     List empty\n");
        hline_log();
        return;
    }

    if (ptr_ok){
        printf_log("    Head: %lu Tail %lu Capacity %lu Size %lu\n", lst->next[0], lst->prev[0], lst->capacity, lst->size);
        printf_log("    Free mem ptr: Stack: %lu Unused end: %lu\n", lst->fmem_stack, lst->fmem_end);
    }

    if (err & VAR_BADSTATE){
        printf_log("     (BAD) List in invalid state (indexes out of range)\n");
    }
    #ifndef LIST_NOCANARY
        if (err & VAR_DATA_CANARY_L_BAD){
            printf_log("     Left data canary bad (D%X-P%X-N%X | %X)\n",
                        (canary_t*)(lst->data+1)[-1], (canary_t*)(lst->data+1)[-1], (canary_t*)(lst->data+1)[-1], CANARY_L);
        }
        if (err & VAR_DATA_CANARY_R_BAD){
            size_t cap = lst->capacity;
            printf_log("     Right data canary bad (D%X-P%X-N%X | %X)\n",
                       (canary_t*)(lst->data+1)[-1], (canary_t*)(lst->data+1)[-1], (canary_t*)(lst->data+1)[-1], CANARY_R);
        }
    #endif


    if (graph_dump){
        listGraphDump(lst);
    }
    else{
        listTextDump(lst);
    }
    hline_log();
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

    if (new_capacity < lst->fmem_end - 1){
        return VAR_BADOP;
    }

    if (allocOrResize((void**)&(lst->data), (new_capacity+1)*sizeof(LIST_ELEM_T) + LIST_DATA_SIZE_OFFSET , LIST_DATA_BEGIN_OFFSET) &&
       allocOrResize((void**)&(lst->prev) , (new_capacity+1)*sizeof(size_t)      + LIST_DATA_SIZE_OFFSET , LIST_DATA_PTR_BEGIN_OFFSET) &&
       allocOrResize((void**)&(lst->next) , (new_capacity+1)*sizeof(size_t)      + LIST_DATA_SIZE_OFFSET , LIST_DATA_PTR_BEGIN_OFFSET)
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
    else {
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
    if (lst->fmem_stack == 0){
        if(lst->fmem_end <= lst->capacity)
            return lst->fmem_end++;
        else
            return 0;
    }
    else {
        size_t t = lst->fmem_stack;
        lst->fmem_stack = lst->next[t];
        return t;
    }
}

size_t listPushAfter(List* lst, size_t ind, LIST_ELEM_T elem, varError_t* err_ptr){
    listCheckRetPtr(lst, err_ptr, 0);

    if(ind >= lst->fmem_end)
        return VAR_BADOP;
    if(lst->prev != nullptr && lst->prev[ind] == ind)
        return VAR_BADOP;

    varError_t err = VAR_NOERROR;

    size_t ni = listGetFreeMem(lst);

    if (ni == 0){
        int new_cap = lst->capacity * 2;
        if (new_cap < 10){
            new_cap = 10;
        }

        err = listResize(lst, new_cap);
        if (err != VAR_NOERROR)
            passError(varError_t, err);

        ni = listGetFreeMem(lst);
        if (ni == 0){
            if (err_ptr){
                *err_ptr = VAR_ERRUNK;
                return 0;
            }
        }
    }

    if(ind != lst->prev[0]){
        lst->sorted = false;
    }

    lst->size++;

    lst->data[ni] = elem;
    lst->prev[ni] = ind;

    lst->next[ni] = lst->next[ind];
    lst->prev[lst->next[ind]] = ni;
    lst->next[ind] = ni;

    return ni;
}

varError_t listDeleteElem(List* lst, size_t ind){
    listCheckRet(lst, listError_dbg(lst));

    if(ind >= lst->fmem_end)
        return VAR_BADOP;
    if(lst->prev == nullptr || lst->prev[ind] == ind)
        return VAR_BADOP;

    if (ind != lst->prev[0]){
        lst->sorted = false;
    }

    if (ind == 0 || ind >= lst->fmem_end || lst->prev[ind] == ind){
        return VAR_BADOP;
    }

    lst->size--;
    lst->next[lst->prev[ind]] = lst->next[ind];

    lst->prev[lst->next[ind]] = lst->prev[ind];

    listAddFreeMem(lst, ind);
    return VAR_NOERROR;
}

varError_t listSerialize(List* lst, size_t new_size){
    listCheckRet(lst, listError_dbg(lst));
    LIST_ELEM_T* new_data = nullptr;
    size_t* new_prev = nullptr;
    size_t* new_next = nullptr;

    if (new_size < lst->size){
        return VAR_BADOP;
    }

    if (allocOrResize((void**)&(new_data), (new_size+1)*sizeof(LIST_ELEM_T) + LIST_DATA_SIZE_OFFSET , LIST_DATA_BEGIN_OFFSET)) {
        int ni = 1;
        int oi = lst->next[0];
        while (ni <= lst->size && oi != 0){
            new_data[ni] = lst->data[oi];
            oi = lst->next[oi];
            ni++;
        }
        if (ni <= lst->size || oi != 0){
            free(new_data);
            return VAR_CORRUPT;
        }

        if (allocOrResize((void**)(&new_prev), (new_size+1)*sizeof(size_t) + LIST_DATA_PTR_SIZE_OFFSET , LIST_DATA_PTR_BEGIN_OFFSET) &&
            allocOrResize((void**)(&new_next), (new_size+1)*sizeof(size_t) + LIST_DATA_PTR_SIZE_OFFSET , LIST_DATA_PTR_BEGIN_OFFSET)   ) {

            free(((char*)lst->data) - LIST_DATA_BEGIN_OFFSET);
            free(((char*)lst->prev) - LIST_DATA_PTR_BEGIN_OFFSET);
            free(((char*)lst->next) - LIST_DATA_PTR_BEGIN_OFFSET);

            lst->data = new_data;
            lst->prev = new_prev;
            lst->next = new_next;
            lst->capacity = new_size;
            lst->sorted = true;

            lst->fmem_end = lst->size + 1;
            lst->fmem_stack = 0;

            for(int i = 1; i <= lst->size; i++){
                new_prev[i  ] = i-1;
                new_next[i-1] = i;
            }
            new_next[lst->size] = 0;
            new_prev[0        ] = lst->size;

            listReplaceDataCanary(lst);
            return VAR_NOERROR;
        }
    }
    if (new_prev != nullptr)
        free(new_prev);
    if (new_next != nullptr)
        free(new_next);
    if (new_data != nullptr)
        free(new_data);
    return VAR_INTERR;
}

