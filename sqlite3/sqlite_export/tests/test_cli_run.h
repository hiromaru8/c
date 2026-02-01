#ifndef TEST_CLI_RUN_H
#define TEST_CLI_RUN_H

int create_test_db(const char* path);

int test_cli_composite_pk_values(void);
int test_cli_composite_pk_all(void);
int test_cli_single_pk_values(void);
int test_cli_single_pk_all(void);

#endif
