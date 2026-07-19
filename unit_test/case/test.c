

# include <stdio.h>
# include <stdint.h>
# include <my_assert.h>
# include <stdbool.h>


/* テストデータ */
static const uint8_t data_no_dup[] = {
    1, 2, 3, 4
};

static const uint8_t data_dup[] = {
    1, 2, 3, 1
};

static const uint8_t data_all_same[] = {
    9, 9, 9, 9
};

static const uint8_t data_single[] = {
    5
};

static const ByteTestCase test_cases[] = {
    {
        .name = "no duplicate",
        .data = data_no_dup,
        .size = sizeof(data_no_dup),
        .expected = false
    },
    {
        .name = "has duplicate",
        .data = data_dup,
        .size = sizeof(data_dup),
        .expected = true
    },
    {
        .name = "all same",
        .data = data_all_same,
        .size = sizeof(data_all_same),
        .expected = true
    },
    {
        .name = "single element",
        .data = data_single,
        .size = sizeof(data_single),
        .expected = false
    }
};

static int test_has_duplicate_bytes(void)
{
    size_t i;

    for (i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {

        bool actual = false;

        if (actual != test_cases[i].expected) {

            printf(
                "FAILED: %s\n"
                " expected=%d actual=%d\n",
                test_cases[i].name,
                test_cases[i].expected,
                actual
            );

            ASSERT_TRUE(false);
        }

        printf("PASSED: %s\n", test_cases[i].name);
    }
    return 0;
}

int main(void)
{
    test_has_duplicate_bytes();
    return 0;
}

/*
gcc -std=c17 -o test test.c
*/