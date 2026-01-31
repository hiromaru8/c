#include <stdio.h>

/* テスト関数宣言 */
int test_composite_pk_with_values(void);
int test_composite_pk_all_rows(void);
int test_single_pk_with_values(void);
int test_single_pk_all_rows(void);

int main(void) {
    int failed = 0;

    failed += test_composite_pk_with_values();
    failed += test_composite_pk_all_rows();
    failed += test_single_pk_with_values();
    failed += test_single_pk_all_rows();

    printf("\n==== TEST RESULT ====\n");
    if (failed == 0) {
        printf("ALL TESTS PASSED\n");
        return 0;
    } else {
        printf("%d TEST(S) FAILED\n", failed);
        return 1;
    }
}
