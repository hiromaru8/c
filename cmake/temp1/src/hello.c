#include <stdio.h>

#ifdef APP_VERSION
#define VERSION APP_VERSION
#else
#define VERSION "0.0.0" // バージョン情報を定義
#endif

int main() {
    printf("Hello, World!\n");
    printf("Version: %s\n", VERSION);
    return 0;
}


