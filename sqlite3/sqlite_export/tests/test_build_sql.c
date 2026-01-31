#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/common.h"

/*
 * 文字列比較用アサート
 */
static int assert_str_eq(
    const char* test_name,
    const char* actual,
    const char* expected
) {
    if (strcmp(actual, expected) != 0) {
        printf("[FAIL] %s\n", test_name);
        printf("  expected: %s\n", expected);
        printf("  actual  : %s\n", actual);
        return 1;
    }
    printf("[ OK ] %s\n", test_name);
    return 0;
}

// テストケース
int test_composite_pk_with_values(void) {
    char sql[1024];

    build_sql(
        "tb1",
        "id_src,id_dest",
        "(1,10),(2,20)",
        "data1,data2",
        sql,
        sizeof(sql)
    );

    return assert_str_eq(
        "composite pk + values",
        sql,
        "SELECT data1,data2 FROM tb1 "
        "WHERE (id_src,id_dest) IN ((1,10),(2,20)) "
        "ORDER BY id_src,id_dest ASC;"
    );
}

int test_composite_pk_all_rows(void) {
    char sql[1024];

    build_sql(
        "tb1",
        "id_src,id_dest",
        NULL,
        "*",
        sql,
        sizeof(sql)
    );

    return assert_str_eq(
        "composite pk + all rows",
        sql,
        "SELECT * FROM tb1 "
        "ORDER BY id_src,id_dest ASC;"
    );
}

int test_single_pk_with_values(void) {
    char sql[1024];

    build_sql(
        "tb2",
        "id",
        "(1),(5),(9)",
        "data1",
        sql,
        sizeof(sql)
    );

    return assert_str_eq(
        "single pk + values",
        sql,
        "SELECT data1 FROM tb2 "
        "WHERE (id) IN ((1),(5),(9)) "
        "ORDER BY id ASC;"
    );
}

int test_single_pk_all_rows(void) {
    char sql[1024];

    build_sql(
        "tb2",
        "id",
        "",
        "*",
        sql,
        sizeof(sql)
    );

    return assert_str_eq(
        "single pk + all rows",
        sql,
        "SELECT * FROM tb2 "
        "ORDER BY id ASC;"
    );
}

