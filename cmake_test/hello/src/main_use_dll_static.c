#include <windows.h>

#include "hello.h"
#include "..\..\libhello\include\libhello.h"

int main(void){
    hello();
    libhello();

    return 0;    
}