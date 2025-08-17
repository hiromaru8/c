#include <stdio.h>
#include "libhello.h"


DLL_EXPORT void libhello(void) {
    printf("Hello World! from C library\n");
}

