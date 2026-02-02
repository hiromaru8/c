#ifndef CLI_RUN_H
#define CLI_RUN_H

#ifdef __cplusplus
extern "C" {
#endif

int cli_run_export(
    const char* db_path,
    const char* table,
    const char* pk,
    const char* pk_values,
    const char* cols,
    const char* out_path
);

int cli_run_update(
    const char* db_path,
    const char* table,
    const char* pk,
    const char* pk_values,
    const char* set
);

#ifdef __cplusplus
}
#endif

#endif /* CLI_RUN_H */
