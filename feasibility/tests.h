#include <stdint.h>

#define NUM_TEST_CASES  12

typedef struct _test {
    int      num_services;
    uint32_t *periods;
    uint32_t *wcets;
} s_test_case;

void print_test_case(int num_services, s_test_case *test_case, int test_num);
