#include <iostream>
#include "MemoryTools.h"

//this is just a simple example of how to use this, you can do many more things with it

int main(int argc, char *argv[]) {
    initXMemoryTools(
        /*process name*/
        "my.process.package",
        /*mode root/no root*/
        MODE_NO_ROOT
        /*MODE_ROOT - for root*/
    );

/*
Value types:

TYPE_DWORD,
TYPE_FLOAT,
TYPE_DOUBLE,
TYPE_WORD,
TYPE_BYTE,
TYPE_QWORD
*/

/*
Search Ranges:

ALL,
B_BAD,
C_ALLOC,
C_BSS,
C_DATA,
C_HEAP,
JAVA_HEAP,
A_ANONYMOUS,
CODE_SYSTEM,
STACK,
ASHMEM
*/

    SetSearchRange(ALL);
    MemorySearch("1"/*value*/, TYPE_DWORD);
    MemoryOffset("2"/*value*/, 2/*offset*/, TYPE_DWORD);
    MemoryWrite("0"/*value*/, 2/*offset*/, TYPE_DWORD);
    ClearResults();
    
    return 0;
}
