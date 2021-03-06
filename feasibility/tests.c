#include <stdio.h>
#include "tests.h"

s_test_case test_cases[NUM_TEST_CASES] = {

{
// sched-example-0-safe-within-LUB-disharmonic.xlsx
// U=0.7333
3,
(uint32_t []){2, 10, 15},
(uint32_t []){1, 1, 2}
},

{
// sched-example-1-above-LUB-failure-disharmonic.xlsx
// U=0.9857
3,
(uint32_t []){2, 5, 7},
(uint32_t []){1, 1, 2}
},

{
// sched-example-2-above-LUB-failure-disharmonic.xlsx
// U=0.9967
4,
(uint32_t []){2, 5, 7, 13},
(uint32_t []){1, 1, 1, 2}
},

{
// sched-example-3-above-LUB-harmonic.xlsx
// U=0.93
3,
(uint32_t []){3, 5, 15},
(uint32_t []){1, 2, 3}
},

{
// sched-example-4-above-LUB-harmonic.xlsx
// U=1.0
3,
(uint32_t []){2, 4, 16},
(uint32_t []){1, 1, 4}
},

{
// sched-example-5-above-LUB-harmonic.xlsx 
// U=1.0
3,
(uint32_t []){2, 5, 10},
(uint32_t []){1, 2, 1}
},

{
// sched-example-6-above-LUB-RM-failure-DM-success-disharmonic.xlsx 
// U=99.29
4,
(uint32_t []){2, 5, 7, 13},
(uint32_t []){1, 1, 1, 2}
},

{
// sched-example-7-above-LUB-harmonic-EDF-and-LLF-difference.xlsx 
// U=100
3,
(uint32_t []){3, 5, 15},
(uint32_t []){1, 2, 4}
},

{
// sched-example-8-above-LUB-failure-disharmonic.xlsx 
// U=99.67
4,
(uint32_t []){2, 5, 7, 13},
(uint32_t []){1, 1, 1, 2}
},

{
// sched-example-9-above-LUB-harmonic.xlsx 
// U=100
4,
(uint32_t []){6, 8, 12, 24},
(uint32_t []){1, 2, 4, 6}
},

{
// sched-example-10-above-LUB-disharmonic.xlsx
// U=98.57
4,
(uint32_t []){2, 5, 7, 14},
(uint32_t []){1, 1, 1, 2}
},

{
// 11 - EDF different vs LLF
// U=100
3,
(uint32_t []){3, 6, 9},
(uint32_t []){1, 2, 3}
},

{
// 12 - Exam question #18 
// U=100
4,
(uint32_t []){6, 8, 12, 24},
(uint32_t []){1, 2, 4, 6}
},

{
// 13 - Exam question # 20
// U=100
4,
(uint32_t []){4, 8, 12, 16},
(uint32_t []){1, 2, 3, 4}
},

{
// 14 - Exam question # 23
// U=100
3,
(uint32_t []){2, 5, 10},
(uint32_t []){1, 1, 3}
},

{
// 15 
// U=100
3,
(uint32_t []){2, 5, 10},
(uint32_t []){1, 2, 3}
},

{
// 16 - First cut Final project 
// U=100
3,
(uint32_t []){10, 40, 100},
(uint32_t []){1, 33, 72}
},

{
// 17 - first cut Final project 
// U=100
3,
(uint32_t []){10, 40, 100},
(uint32_t []){1, 33, 72}
},

{
// 18 - Final project 2 services
// U=100
2,
(uint32_t []){10, 40},
(uint32_t []){1, 33}
},

{
// 19 - second cut Final project 
// U=100
3,
(uint32_t []){10, 40, 100},
(uint32_t []){1, 33, 6}
}

};

void print_test_case(int num_services, s_test_case *test_case, int test_num) {

        float utility = 0;
        for (int service=0; service < num_services; service++) {
            utility += (float)test_case->wcets[service] / (float)test_case->periods[service];
        }

        printf("Ex-%0d U=%4.2f (", test_num, utility);
        for (int service=0; service < num_services; service++) {
            printf("C%0d=%d", service + 1, test_case->wcets[service]);
            printf("%s",  (service == num_services -1) ? "; " : ", ");
        }

        for (int service=0; service < num_services; service++) {
            printf("T%0d=%d", service + 1 , test_case->periods[service]);
            printf("%s",  (service == num_services -1) ? "; " : ", ");
        }

       printf("T=D): ");
}
