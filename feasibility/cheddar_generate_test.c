#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tests.h"

void print_header(char* protocol);


int main(int argc, char* argv[]) {


if (argc !=2) {
    printf("usage: cheddar_generate_test <protocol#>\n");
    exit(0);
}


char* protocol = NULL;

    if (strcmp(argv[1], "RM") == 0) {
        protocol = "RATE_MONOTONIC_PROTOCOL";
    }

    if (strcmp(argv[1], "EDF") == 0) {
        protocol = "EARLIEST_DEADLINE_FIRST_PROTOCOL";
    }


    if (protocol == NULL) {
        printf("error -no such protocol\n");
        exit(-1);
    }

printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>");


printf("<cheddar>\n");
print_header(protocol);

printf("</cheddar>\n");




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
