#include "tests.h"

s_test_case test_cases[10] = {

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

};
