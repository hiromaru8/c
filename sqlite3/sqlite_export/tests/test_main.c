#include "test_cli_run.h"
#include <stdio.h>

/* テスト関数宣言 */
int test_composite_pk_with_values(void);
int test_composite_pk_all_rows(void);
int test_single_pk_with_values(void);
int test_single_pk_all_rows(void);


static const char* TEST_DB = "test.db";
static const char* OUT_BINARY_FILE = "out.bin";
/* ===== main : setup / run / teardown ===== */
int main(void) {
    int failed = 0;

    /* ---- SETUP ---- */
    remove(TEST_DB);
    if (create_test_db(TEST_DB) != 0) {
        fprintf(stderr, "DB setup failed\n");
        return 1;
    }


    /* ---- RUN TESTS ---- */
    // 各テスト関数を呼び出し
    failed += test_composite_pk_with_values();
    failed += test_composite_pk_all_rows();
    failed += test_single_pk_with_values();
    failed += test_single_pk_all_rows();


    // cli_run.c のテストもここで実行
    failed += test_cli_composite_pk_values();
    failed += test_cli_composite_pk_all();
    failed += test_cli_single_pk_values();
    failed += test_cli_single_pk_all();
    remove(OUT_BINARY_FILE);
    remove(TEST_DB);

    printf("\n==== TEST RESULT ====\n");
    if (failed == 0) {
        printf("ALL TESTS PASSED\n");
        return 0;
    } else {
        printf("%d TEST(S) FAILED\n", failed);
        return 1;
    }
}
