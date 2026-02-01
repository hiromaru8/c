#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/common.h"

/*
 * 文字列比較用アサート
 */
#define ASSERT_EQ_STR(exp, act) \
    do { \
        if (strcmp((exp), (act)) != 0) { \
            printf("ASSERT_EQ_STR FAILED: %s=\"%s\", %s=\"%s\" (%s:%d)\n", \
                   #exp, (exp), #act, (act), __FILE__, __LINE__); \
            return 1; \
        } \
    } while (0)



// *************************************************
// テストケース
// *************************************************
// 複合主キー + PK値指定
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

    ASSERT_EQ_STR(
        "SELECT data1,data2 FROM tb1 "
        "WHERE (id_src,id_dest) IN ((1,10),(2,20)) "
        "ORDER BY id_src,id_dest ASC;",
        sql
    );

    return 0;
}

// 複合主キー + 全件
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

    ASSERT_EQ_STR(
        "SELECT * FROM tb1 "
        "ORDER BY id_src,id_dest ASC;",
        sql
    );
}

// 単一主キー + PK値指定
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
    ASSERT_EQ_STR(
        "SELECT data1 FROM tb2 "
        "WHERE (id) IN ((1),(5),(9)) "
        "ORDER BY id ASC;",
        sql
    );

    return 0;
}

// 単一主キー + PK値指定
int test_single_pk_with_values2(void) {
    char sql[1024];

    build_sql(
        "tb2",
        "id",
        "(1),(5),(9)",
        "data1",
        sql,
        sizeof(sql)
    );
    ASSERT_EQ_STR(
        "SELECT data1 FROM tb2 "
        "WHERE (id) IN ((1),(5),(9)) "
        "ORDER BY id ASC;",
        sql
    );
    return 0;
}

// 単一主キー + 全件
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

    ASSERT_EQ_STR(
        "SELECT * FROM tb2 "
        "ORDER BY id ASC;",
        sql
    );
    return 0;
}