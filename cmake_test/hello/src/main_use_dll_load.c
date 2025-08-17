#include <windows.h>
#include <stdio.h>

typedef void (*libhello_func)(void);

int main(void) {
    HMODULE hDLL = LoadLibraryA("libhello.dll"); // DLL を読み込む
    if (hDLL == NULL) {
        printf("failed to load DLL\n");
        return 1;
    }

    // 関数ポインタを取得
    libhello_func libhello = (libhello_func)GetProcAddress(hDLL, "libhello");
    if (libhello == NULL) {
        printf("failed to get function address\n");
        FreeLibrary(hDLL);
        return 1;
    }

    // 関数呼び出し
    libhello();

    // DLL 解放
    FreeLibrary(hDLL);
    return 0;
}
