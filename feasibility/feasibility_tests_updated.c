#include <stdint.h>
#include <math.h>
#include <stdio.h>

#define TRUE 1
#define FALSE 0
#define uint32_t unsigned int


typedef struct _test {
    int      num_services;
    uint32_t *periods;
    uint32_t *wcets;
} s_test_case;

s_test_case test_cases[5] = {

{
// U=0.7333
3,
(uint32_t []){2, 10, 15},
(uint32_t []){1, 1, 2}
},

{
// U=0.9857
3,
(uint32_t []){2, 5, 7},
(uint32_t []){1, 1, 2}
},

{
// U=0.9967
4,
(uint32_t []){2, 5, 7, 13},
(uint32_t []){1, 1, 1, 2}
},

{
// U=0.93
3,
(uint32_t []){3, 5, 15},
(uint32_t []){1, 2, 3}
},

{
// U=1.0
3,
(uint32_t []){2, 4, 16},
(uint32_t []){1, 1, 4}
}
};


void print_test_case(int num_services, s_test_case *test_case, int test_num);
int completion_time_feasibility(uint32_t numServices, uint32_t period[], uint32_t wcet[]);
int scheduling_point_feasibility(uint32_t numServices, uint32_t period[], uint32_t wcet[]);


int main(void)
{
    uint32_t numServices;

    printf("******** Completion Test Feasibility Example\n");

    for (int test_case=0; test_case < sizeof(test_cases)/sizeof(s_test_case); test_case++) {

        //TODO - force test case writer to make periods[], wcet[] same size
        int num_services = test_cases[test_case].num_services;

        print_test_case(num_services, &test_cases[test_case], test_case);
            
        if(completion_time_feasibility(num_services, test_cases[test_case].periods, test_cases[test_case].wcets) == TRUE)
            printf("FEASIBLE\n");
        else
            printf("INFEASIBLE\n");
    }


    printf("\n\n");
    printf("******** Scheduling Point Feasibility Example\n");

    for (int test_case=0; test_case < sizeof(test_cases)/sizeof(s_test_case); test_case++) {

        //TODO - force test case writer to make periods[], wcet[] same size
        int num_services = test_cases[test_case].num_services;

        print_test_case(num_services, &test_cases[test_case], test_case);
            
        if(scheduling_point_feasibility(num_services, test_cases[test_case].periods, test_cases[test_case].wcets) == TRUE)
            printf("FEASIBLE\n");
        else
            printf("INFEASIBLE\n");
    }
}


int completion_time_feasibility(uint32_t numServices, uint32_t period[], uint32_t wcet[])
{
  int i, j;
  uint32_t an, anext;

  uint32_t *deadline = period;

  // assume feasible until we find otherwise
  int set_feasible=TRUE;

  //printf("numServices=%d\n", numServices);

  for (i=0; i < numServices; i++)
  {
       an=0; anext=0;

       for (j=0; j <= i; j++)
       {
           an+=wcet[j];
       }

       //printf("i=%d, an=%d\n", i, an);

       while(1)
       {
             anext=wcet[i];

             for (j=0; j < i; j++)
                 anext += ceil(((double)an)/((double)period[j]))*wcet[j];

             if (anext == an)
                break;
             else
                an=anext;

             //printf("an=%d, anext=%d\n", an, anext);
       }

       //printf("an=%d, deadline[%d]=%d\n", an, i, deadline[i]);

       if (an > deadline[i])
       {
          set_feasible=FALSE;
       }
  }

  return set_feasible;
}


int scheduling_point_feasibility(uint32_t numServices, uint32_t period[],
                                uint32_t wcet[])
{
   int rc = TRUE, S2, S, i, num_periods, status, temp;

   for (S2=0; S2 < numServices; S2++) // iterate from highest to lowest priority
   {
      status=0;

      for (S=0; S<=S2; S++) // S = service whose periods must be analyzed
      {
          // num_periods to ananlyze for S
          int floor_T2_T1 = floor((double)period[S2]/(double)period[S]);
          for (num_periods=1; num_periods <= floor_T2_T1; num_periods++)
          {
               temp=0;

               int ceiling_T1_Tn = ceil((double)num_periods*(double)period[S]/(double)period[i]);
               // i = service between S1..Sn
               for (i=0; i<=S2; i++)
               {
                   int ceiling_T1_Tn = ceil((double)num_periods*(double)period[S]/(double)period[i]);
                   temp += wcet[i] * ceiling_T1_Tn;
               }

               if (temp <= (num_periods*period[S]))
               {
                   status=1;
                   break;
               }
           }
           if (status) break;
      }
      if (!status) rc=FALSE;
   }
   return rc;
}

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
