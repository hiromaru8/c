#ifndef TEST_CASE_H
#define TEST_CASE_H


/**
 * ByteTestCase - テストケースごとのテストデータと期待される結果を表す構造体
 * この構造体は、バイトデータを表すために使用されます。
 */
typedef struct {
    const char *name;       /* テストケース名 */
    const uint8_t *data;    /* テストデータ */
    size_t size;            /* テストデータのサイズ */
    bool expected;          /* 期待される結果: trueなら重複あり、falseなら重複なし */
} ByteTestCase;


typedef struct {
    const uint8_t *data;
    size_t size;
} ByteArray;

typedef struct {
    bool has_duplicate;
} DuplicateExpected;

typedef struct {
    const char *name;
    ByteArray input;
    bool boolean_expected;          /* 期待される結果: bool */
    int  int_expected;              /* 期待される結果: int */
} ByteDuplicateTestCase;


#endif /* TEST_CASE_H */
