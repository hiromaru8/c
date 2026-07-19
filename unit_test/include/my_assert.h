#ifndef MY_ASSERT_H
#define MY_ASSERT_H

#include <stdio.h>

/*
 * Simple assertion macros for unit testing in C.
 * このマクロは、C言語での単体テストのための簡単なアサーションマクロです。
 * 使い方:
 * ASSERT_EQ_INT(expected, actual) - 期待される整数値と実際の整数値が等しいことを確認します。
 * ASSERT_TRUE(condition) - 条件が真であることを確認します。
 * これらのマクロは、条件が満たされない場合にエラーメッセージを出力し、テスト関数から1を返します。
 * 例: 
 * int test_example() {
 *     int result = some_function();
 *    ASSERT_EQ_INT(42, result);
 *    return 0;
 * }
 */
#define ASSERT_EQ_INT(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf("[FAIL] %s:%d expected=%d actual=%d\n", \
                   __FILE__, __LINE__, \
                   (expected), (actual)); \
            return 1; \
        } \
    } while (0)


/*
 * ASSERT_TRUE(condition) - 条件が真であることを確認します。
 * 例:
 * int test_example() {
 *     int value = some_function();
 *     ASSERT_TRUE(value > 0);
 *     return 0;
 * }
 */
#define ASSERT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s:%d condition failed\n", \
                   __FILE__, __LINE__); \
            return 1; \
        } \
    } while (0)

/*
 * ASSERT_FALSE(condition) - 条件が偽であることを確認します。
 * 例:
 * int test_example() {
 *     int value = some_function();
 *     ASSERT_FALSE(value < 0);
 *     return 0;
 * }
 */
#define ASSERT_FALSE(cond) \
    do { \
        if (cond) { \
            printf("[FAIL] %s:%d condition should be false\n", \
                   __FILE__, __LINE__); \
            return 1; \
        } \
    } while (0)



#endif /* MY_ASSERT_H */
