#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tests.h"

#define STARTING_TASK_ID    10

void print_header(char* protocol);
void print_task(int id, int capacity, int period, int deadline);

extern s_test_case test_cases[];

int main(int argc, char* argv[]) {


if (argc !=3) {
    printf("usage: cheddar_generate_test <protocol#> <test_case#>\n");
    exit(0);
}

int test_case = atoi(argv[2]);
if (test_case > NUM_TEST_CASES) {
    printf("Only %d - %d test cases available\n", 0, NUM_TEST_CASES - 1);
    exit(0);
}

//TODO - force test case writer to make periods[], wcet[] same size
int num_services = test_cases[test_case].num_services;

char* protocol = NULL;

    if (strcmp(argv[1], "RM") == 0) {
        protocol = "RATE_MONOTONIC_PROTOCOL";
    }

    if (strcmp(argv[1], "EDF") == 0) {
        protocol = "EARLIEST_DEADLINE_FIRST_PROTOCOL";
    }

    if (strcmp(argv[1], "LLF") == 0) {
        protocol = "LEAST_LAXITY_FIRST_PROTOCOL";
    }


    if (protocol == NULL) {
        printf("error -no such protocol\n");
        exit(-1);
    }

printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>");


printf("<cheddar>\n");
print_header(protocol);


printf("<tasks>\n");

for (int service=0; service < num_services; service++) {
    int capacity = test_cases[test_case].wcets[service];
    int period = test_cases[test_case].periods[service];
    int deadline = period;
    print_task(service, capacity, period, deadline);
}

printf("</tasks>\n");
printf("</cheddar>\n");

printf("<!-- ");
print_test_case(num_services, &test_cases[test_case], test_case);
printf("-->\n\n");



}

void print_header(char* protocol) {

printf(" "
" <core_units> \n"
"  <core_unit id=\"id_1\"> \n"
"   <object_type>CORE_OBJECT_TYPE</object_type> \n"
"   <name>CORE</name> \n"
"   <scheduling> \n"
"    <scheduling_parameters> \n"
"     <scheduler_type>%s</scheduler_type> \n" // Insert protocol here
"     <quantum>0</quantum> \n"
"     <preemptive_type>PREEMPTIVE</preemptive_type> \n"
"     <capacity>0</capacity> \n"
"     <period>0</period> \n"
"     <priority>0</priority> \n"
"     <start_time>0</start_time> \n"
"    </scheduling_parameters> \n"
"   </scheduling> \n"
"   <speed>0.00000</speed> \n"
"  </core_unit> \n"
" </core_units> \n"
" <processors> \n"
"  <mono_core_processor id=\"id_2\"> \n"
"   <object_type>PROCESSOR_OBJECT_TYPE</object_type> \n"
"   <name>CPU_0</name> \n"
"   <processor_type>MONOCORE_TYPE</processor_type> \n"
"   <migration_type>NO_MIGRATION_TYPE</migration_type> \n"
"   <core ref=\"id_1\"> \n"
"   </core> \n"
"  </mono_core_processor> \n"
" </processors> \n"
" <address_spaces> \n"
"  <address_space id=\"id_3\"> \n"
"   <object_type>ADDRESS_SPACE_OBJECT_TYPE</object_type> \n"
"   <name>ADDRESS_0</name> \n"
"   <cpu_name>CPU_0</cpu_name> \n"
"   <text_memory_size>0</text_memory_size> \n"
"   <stack_memory_size>0</stack_memory_size> \n"
"   <data_memory_size>0</data_memory_size> \n"
"   <heap_memory_size>0</heap_memory_size> \n"
"   <scheduling> \n"
"    <scheduling_parameters> \n"
"     <scheduler_type>NO_SCHEDULING_PROTOCOL</scheduler_type> \n"
"     <quantum>0</quantum> \n"
"     <preemptive_type>PREEMPTIVE</preemptive_type> \n"
"     <capacity>0</capacity> \n"
"     <period>0</period> \n"
"     <priority>0</priority> \n"
"     <start_time>0</start_time> \n"
"    </scheduling_parameters> \n"
"   </scheduling> \n"
"  </address_space> \n"
" </address_spaces> \n",
protocol);

}

void print_task(int id, int capacity, int period, int deadline) {
printf(""
"<periodic_task id=\"id_%d\"> \n" //id
"   <object_type>TASK_OBJECT_TYPE</object_type> \n"
"   <name>S%d</name> \n" //name
"   <task_type>PERIODIC_TYPE</task_type> \n"
"   <cpu_name>CPU_0</cpu_name> \n"
"   <address_space_name>ADDRESS_0</address_space_name> \n"
"   <capacity>%d</capacity> \n" //capacity
"   <deadline>%d</deadline> \n" //deadline
"   <start_time>0</start_time> \n"
"   <priority>1</priority> \n"
"   <blocking_time>0</blocking_time> \n"
"   <policy>SCHED_FIFO</policy> \n"
"   <text_memory_size>0</text_memory_size> \n"
"   <stack_memory_size>0</stack_memory_size> \n"
"   <criticality>0</criticality> \n"
"   <context_switch_overhead>0</context_switch_overhead> \n"
"   <period>%d</period> \n" //period
"   <jitter>0</jitter> \n"
"   <every>0</every> \n"
"  </periodic_task> \n",
id + STARTING_TASK_ID , id + 1, capacity, deadline, period);

}
