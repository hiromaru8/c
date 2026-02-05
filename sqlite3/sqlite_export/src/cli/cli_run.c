




/*
 * cli_run.c
 *
 * コマンドライン EXE の処理本体
 */
#include "cli_run.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cli_run_export(
    const char* db_path,
    const char* table,
    const char* pk,
    const char* pk_values,
    const char* cols,
    const char* out
) {

    char sql[1024];
    build_select_sql(table, pk, pk_values, cols, sql, sizeof(sql));

    printf("SQL: %s\n", sql);

    unsigned char* buf;
    int size;
    int rc;
    /* DB Open*/
    sqlite3* db = NULL;
    rc = open_database_readonly(db_path, &db);
    if (rc != 0) {
        printf("DB open failed (rc=%d)\n", rc);
        return 1;
    }


    /* DB からデータ取得 */
    rc = exec_query_to_buffer(db, sql, &buf, &size);
    if (rc != 0) {
        printf("Query failed (rc=%d)\n", rc);
        close_database(db);
        return 2;
    }
    if(buf == NULL || size <= 0) {
        printf("No data retrieved\n");
        close_database(db);
        return 3;
    }
    close_database(db);

    /* バイナリファイルへ書き込み */
    FILE* fp = fopen(out, "wb");
    fwrite(buf, 1, size, fp);
    fclose(fp);

    free(buf);

    printf("Exported %d bytes\n", size);
    return 0;

}

int cli_run_update(
    const char* db_path,
    const char* table,
    const char* pk,
    const char* pk_values,
    const char* set
) {
    char sql[1024];
    build_update_sql(
        table,
        set,
        pk,
        pk_values,
        sql,
        sizeof(sql)
    );
    printf("SQL: %s\n", sql);

    int rc;
    /* DB Open*/
    sqlite3* db = NULL;
    rc = open_database_readwrite(db_path, &db);
    if (rc != 0) {
        printf("DB open failed (rc=%d)\n", rc);
        return 1;
    }

    /* DB へ更新実行 */
    int affected_rows = 0;
    rc = exec_update(db, sql, &affected_rows);
    if (rc != 0) {
        printf("Update failed (rc=%d)\n", rc);
        close_database(db);
        return 2;
    }

    printf("Update succeeded, affected rows: %d\n", affected_rows);
    close_database(db);
    return 0;
}

