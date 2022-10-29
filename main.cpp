#include <stdio.h>

#include "List.h"


using namespace std;

int main()
{
    List lst;
    listCtor(&lst);
    error_log("this program works\n");
    warn_log ("or it does not\n");

    for(int i = 0; i < 13; i++){
        listPushAfter(&lst, 0, i, nullptr);
    }
    listPushAfter(&lst, 6, 33, nullptr);
    for(int i = 0; i < 13; i += 2){
        listDeleteElem(&lst, i);
    }

    lst.prev[5] = 7;
    header_log("Before sort");
    listDump(&lst);
    listSerialize(&lst, lst.size);
    header_log("After sort");
    listDump(&lst);


    listDtor(&lst);

    return 0;
}
