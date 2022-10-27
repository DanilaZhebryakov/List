#include <stdio.h>

#include "List.h"


using namespace std;

int main()
{
    List lst;
    listCtor(&lst);

    for(int i = 0; i < 13; i++){
        listPushAfter(&lst, 0, i, nullptr);
    }
    //listSerialize(&lst, lst.size);
    listDump(&lst);

    listDtor(&lst);

    return 0;
}
