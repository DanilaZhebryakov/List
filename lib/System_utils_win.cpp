#include <windows.h>

bool isPtrReadable(const void* ptr, size_t size){
    return !IsBadReadPtr(ptr, size);
}

bool isPtrWritable(void* ptr, size_t size){
    return !IsBadWritePtr(ptr, size);
}
