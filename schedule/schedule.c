#include <stdio.h>
#include <assert.h>
#include <math.h>

typedef struct _service {
    //Static service info
    int Tperiod;
    int Cruntime;

    //Dynamic counters
    int Deadline;
    int Runtime;
} Service;

#define SERVICE_NUM(x) (x+1)

int main(int argc, char* argv) {


/* example from sched-example-0-safe for testing
    //Already in rate monotonic order
    Service S1 = { 2, 1 , 0 };
    Service S2 = { 10, 1 , 0 };
    Service S3 = { 15, 2, 0 };
    #define NUM_SERVICES 3
    Service *Services[NUM_SERVICES] = { &S1, &S2, &S3 };
*/

/* example from exercise 1 for testing
    //Already in rate monotonic order
    Service S1 = { 2, 1 , 0 };
    Service S2 = { 5, 2 , 0 };
    Service S3 = { 10, 1, 0 };
    #define NUM_SERVICES 3
    Service *Services[NUM_SERVICES] = { &S1, &S2, &S3 };
*/

/* Exercise 1, homework
*/
    //Already in rate monotonic order
    Service S1 = { 3, 1 , 0 };
    Service S2 = { 5, 2 , 0 };
    Service S3 = { 15, 3, 0 };
    #define NUM_SERVICES 3
    Service *Services[NUM_SERVICES] = { &S1, &S2, &S3 };

/* Exercise 1, part 4 test
    //Already in rate monotonic order
    Service S1 = { 2, 1 , 0 };
    Service S2 = { 5, 2 , 0 };
    #define NUM_SERVICES 2
    Service *Services[NUM_SERVICES] = { &S1, &S2 };
*/

    //Initialize all service's deadlines
    for (int s=0; s < NUM_SERVICES; s++) {
        Services[s]->Deadline = Services[s]->Tperiod;
        Services[s]->Runtime = 0; 
    }

    //TODO - Lowest Common Multiple calculator
    #define LCM 15

    //Record events for easier printout, -1 == nothing happened
    #define TICKS (LCM * 2) // Run two total periods
    int events[TICKS] = { [0 ... (TICKS-1)] = -1 };

    //Statistics

    //Run scheduling engine
    for (int t=0; t < TICKS; t++) {

        //Check for deadline expiration, reset runtime
        for (int s=0; s < NUM_SERVICES; s++) {
            if (Services[s]->Deadline == 0) {
                Services[s]->Deadline = Services[s]->Tperiod;

                if (Services[s]->Runtime == Services[s]->Cruntime) {
                    Services[s]->Runtime = 0;
                } else { //missed deadline
                    printf("!S%d - missed\n", SERVICE_NUM(s));
                    Services[s]->Runtime = 0;
                }
            }
        }

        //Run service with highest priority that still needs runtime
        for (int s=0; s < NUM_SERVICES; s++) {
            if (Services[s]->Runtime < Services[s]->Cruntime) {
                //printf("[ S%d ]", SERVICE_NUM(s)); //Run
                Services[s]->Runtime++;
                events[t] = s;
                break; //Only run this service
            }
        }

        //Clock tick
        for (int s=0; s < NUM_SERVICES; s++) {
            Services[s]->Deadline--;
        }

    }


    printf("Key:\n\t[t]\ttime slice\n\t(Sn)\tservice run\n\t****\tCPU slack\n\t<-Dn>\tDeadline\n\n");
    //Print out schedule
    printf("Schedule:\n");

    //Time ticks banner
    printf("\n");
    for (int t=0; t < TICKS; t++) {
        printf("[%2d]", t + 1);
    }
    printf("\n");

    //Print out each service on a seperate line
    for (int s=0; s < NUM_SERVICES; s++) {
        for (int t=0; t < TICKS; t++) {
            if (events[t] == s) {
                printf("(S%d)", SERVICE_NUM(s));
            } else {
                if (events[t] == -1) { //Nothing happened in this time slice
                    printf("****");
                } else {
                    printf("    ");
                }
            }
        }
        printf("\n");
    }

    
    //Print out deadline chart for each service
    printf("\nDeadlines:\n\n");
    for (int s=0; s < NUM_SERVICES; s++) {
        int deadline = Services[s]->Tperiod;
        for (int t=0; t < TICKS; t++) {
            if (deadline == Services[s]->Tperiod) {
                printf("<");
            }
            deadline--;
            if (deadline == 0) {
                printf("D%d>", SERVICE_NUM(s));
                deadline = Services[s]->Tperiod;
            } else {
                printf("----");
            }
        }
        printf("\n");
    }

    //Statistics
    printf("\nStatistics:\n\n");
    int cpu_usage[NUM_SERVICES] = { [0 ... (NUM_SERVICES-1)] = 0};
    int cpu_slack = 0;

    //Count up cpu statistics
    for (int t=0; t < TICKS; t++) {
        assert(events[t] < NUM_SERVICES); //Sanity check state machine

        if (events[t] == -1) {
            cpu_slack++;
        } else {
            cpu_usage[events[t]]++;
        }
    }

    //Print service cpu usage and total
    float total_cpu = 0;
    for (int s=0; s < NUM_SERVICES; s++) {
        float percent = ((float)cpu_usage[s] / (float)TICKS) * 100;
        printf("S%d    %3d/%3d   %3f\n", SERVICE_NUM(s), cpu_usage[s], TICKS, percent);
        total_cpu += percent;
    }

    printf("                           (%3f) - total service\n", total_cpu);
    float least_upper_bound = NUM_SERVICES * ( pow(2, (float)1 / NUM_SERVICES) - 1) * 100;
    printf("                           (%3f) - Least Upper Bound\n", least_upper_bound);
    printf("                           Schedule is %s\n", total_cpu > least_upper_bound ? "unsafe!" : "safe");

    float percent = ((float)cpu_slack / (float)TICKS) * 100;
    printf("slack %3d/%3d   %3f\n", cpu_slack, TICKS, percent); 
    total_cpu += percent;

    printf("-------------------------\n");
    printf("total           %3f%% cpu\n", total_cpu); 
}
