#include <stdint.h>

int shift_left(int x, int n) {
    return x << n;
}
int shift_left_1(int x) {
    return x << 2;
}

int shift_right(int x, int n) {
    return x >> n; // 算術シフト（符号付き）
}

uint32_t shift_right_logical(uint32_t x, int n) {
    return x >> n; // 論理シフト（符号なし）
}
