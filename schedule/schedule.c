#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "../feasibility/tests.h"
extern s_test_case test_cases[];

typedef struct _service {
    //Static service info
    int Tperiod;
    int Cruntime;

    //Dynamic counters
    int Deadline; //This is TTD + 1
    int Runtime;
} Service;

#define SERVICE_NUM(x) (x+1)

#define PROTO_NONE 0
#define PROTO_RM 1
#define PROTO_EDF 2
#define PROTO_LLF 3

int num_services = 0;

int main(int argc, char* argv[]) {

if (argc != 3) {
    printf("usage: schedule <test#> <RM/EDF/LLF>\n");
    exit(1);
}

int protocol = PROTO_NONE;

if (strcmp(argv[2], "RM") == 0) {
    protocol = PROTO_RM;
}

if (strcmp(argv[2], "EDF") == 0) {
    protocol = PROTO_EDF;
}

if (strcmp(argv[2], "LLF") == 0) {
    protocol = PROTO_LLF;
}


if (protocol == PROTO_NONE) {
    printf("error -no such protocol\n");
    exit(-1);
}


int service = atoi(argv[1]);

num_services = test_cases[service].num_services;

Service* services = malloc(sizeof(Service) * num_services);
assert(services);

for (int s=0; s < num_services; s++) {
    services[s].Tperiod =  test_cases[service].periods[s];
    services[s].Cruntime = test_cases[service].wcets[s];
    services[s].Deadline = services[s].Tperiod;
    services[s].Runtime = 0;
}


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
    //Already in rate monotonic order
    Service S1 = { 3, 1 , 0 };
    Service S2 = { 5, 2 , 0 };
    Service S3 = { 15, 3, 0 };
    #define NUM_SERVICES 3
    Service *Services[NUM_SERVICES] = { &S1, &S2, &S3 };
*/

/* Exercise 1, part 4 test
    //Already in rate monotonic order
    Service S1 = { 2, 1 , 0 };
    Service S2 = { 5, 2 , 0 };
    #define NUM_SERVICES 2
    Service *Services[NUM_SERVICES] = { &S1, &S2 };
*/

    //Initialize all service's deadlines
/*
    for (int s=0; s < NUM_SERVICES; s++) {
        Services[s]->Deadline = Services[s]->Tperiod;
        Services[s]->Runtime = 0; 
    }
*/

    //TODO - Lowest Common Multiple calculator
    #define LCM 15

    //Record events for easier printout, -1 == nothing happened
    #define TICKS (LCM * 2) // Run two total periods
    int events[TICKS] = { [0 ... (TICKS-1)] = -1 };

    //Statistics
    int misses = 0;
    int **STATS_buf = calloc(sizeof(int*),  TICKS); //store [tick][Service LLF]
assert(STATS_buf);
    for (int i=0; i<TICKS; i++) {
        STATS_buf[i] = calloc(sizeof(int), num_services);
assert(STATS_buf[i]);
    }

    //Run scheduling engine
    for (int t=0; t < TICKS; t++) {

        //Check for deadline expiration, reset runtime
        for (int s=0; s < num_services; s++) {
            if (services[s].Deadline == 0) {
                services[s].Deadline = services[s].Tperiod;

                if (services[s].Runtime == services[s].Cruntime) {
                    services[s].Runtime = 0;
                } else { //missed deadline
                    printf("!S%d - missed\n", SERVICE_NUM(s));
                    misses++;
                    services[s].Runtime = 0;
                }
            }
        }

if (protocol == PROTO_RM) {
        //Run service with highest priority that still needs runtime
        for (int s=0; s < num_services; s++) {
            if (services[s].Runtime < services[s].Cruntime) {
                //printf("[ S%d ]", SERVICE_NUM(s)); //Run
                services[s].Runtime++;
                events[t] = s;
                break; //Only run this service
            }
        }
}

if (protocol == PROTO_EDF) {
        //Run service with runtime closest to deadline (EDF)
        int serviceToRun = -1;

        int earliestDeadline = INT_MAX;
        for (int s=0; s < num_services; s++) {

           int running = services[s].Runtime < services[s].Cruntime;
//printf("--> service %d is %d\n", s, running);
           if (running) {
//printf("--> service %d has deadline %d\n", s, services[s]->Deadline);
            if (services[s].Deadline < earliestDeadline) {
                earliestDeadline = services[s].Deadline;
//printf("New lowest deadline %d\n", earliestDeadline);
                serviceToRun = s;
            }
            STATS_buf[t][s] = services[s].Deadline;
           } else {
            STATS_buf[t][s] = -1;
           }
        }

//printf("--> run service %d\n", serviceToRun);
        if (serviceToRun != -1) {
            services[serviceToRun].Runtime++;
            events[t] = serviceToRun;
        }
}

if (protocol == PROTO_LLF) {
        //Run service with runtime closest to deadline (EDF)
        int serviceToRun = -1;

        int leastLaxity = INT_MAX;
        for (int s=0; s < num_services; s++) {

           int running = services[s].Runtime < services[s].Cruntime;
           if (running) {
            int laxity = services[s].Deadline - (services[s].Cruntime - services[s].Runtime);
            if (laxity < leastLaxity) {
                leastLaxity = laxity;
                serviceToRun = s;
            }
            STATS_buf[t][s] = laxity;
           } else {
            STATS_buf[t][s] = -1;
           }
        }

        if (serviceToRun != -1) {
            services[serviceToRun].Runtime++;
            events[t] = serviceToRun;
        }
}


        //Clock tick
        for (int s=0; s < num_services; s++) {
            services[s].Deadline--;
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
    for (int s=0; s < num_services; s++) {
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
    for (int s=0; s < num_services; s++) {
        int deadline = services[s].Tperiod;
        for (int t=0; t < TICKS; t++) {
            if (deadline == services[s].Tperiod) {
                printf("<");
            }
            deadline--;
            if (deadline == 0) {
                printf("D%d>", SERVICE_NUM(s));
                deadline = services[s].Tperiod;
            } else {
                printf("----");
            }
        }
        printf("\n");
    }

if (protocol == PROTO_EDF || protocol == PROTO_LLF) {
    for (int s=0; s < num_services; s++) {
        for (int t=0; t < TICKS; t++) {
            if (STATS_buf[t][s] == -1) {
                printf("% 4s", "X");
            } else {
                printf("% 4d", STATS_buf[t][s]);
            }
        }
        printf(" :S%d\n",s);
    }
}

    //Statistics
    printf("\nStatistics:\n\n");
    int *cpu_usage = malloc(sizeof(int) * num_services);
assert(cpu_usage);

    //int cpu_usage[num_services] = { [0 ... (num_services-1)] = 0};
    int cpu_slack = 0;

    //Count up cpu statistics
    for (int t=0; t < TICKS; t++) {
        assert(events[t] < num_services); //Sanity check state machine

        if (events[t] == -1) {
            cpu_slack++;
        } else {
            cpu_usage[events[t]]++;
        }
    }

    //Print service cpu usage and total
    float total_cpu = 0;
    for (int s=0; s < num_services; s++) {
        float percent = ((float)cpu_usage[s] / (float)TICKS) * 100;
        printf("S%d    %3d/%3d   %3f\n", SERVICE_NUM(s), cpu_usage[s], TICKS, percent);
        total_cpu += percent;
    }

    printf("                           (%3f) - total service\n", total_cpu);
    float least_upper_bound = num_services * ( pow(2, (float)1 / num_services) - 1) * 100;
    printf("                           (%3f) - Least Upper Bound\n", least_upper_bound);
    printf("                           Schedule is %s\n", total_cpu > least_upper_bound ? "unsafe!" : "safe");

    float percent = ((float)cpu_slack / (float)TICKS) * 100;
    printf("slack %3d/%3d   %3f\n", cpu_slack, TICKS, percent); 
    total_cpu += percent;

    printf("-------------------------\n");
    printf("total           %3f%% cpu\n", total_cpu); 

    printf("\nmisses = %d\n", misses);

    printf("Test case #%d: ", service);
    for (int s=0; s < num_services; s++) {
        printf("T%d ", services[s].Tperiod);
    }
    for (int s=0; s < num_services; s++) {
        printf("C%d ", services[s].Cruntime);
    }
    printf("\n");

}
