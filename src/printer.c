#include <stdio.h>
#include <stdlib.h>
#include "../include/printer.h"

void printer_print_proc_usage(AnalysedStats* aStats) {

    if(aStats == NULL) {
        return;
    }
    
    int systemRet = system("clear");
    if(systemRet == -1){
    // The system method failed
    }
    printf("cpu *: ");
    printf("%f%%", (double)aStats->total);
    printf("\n");
    
    for(uint8_t i = 0; i < aStats->cpus_number; i++) {
        printf("cpu %d: ", i);
        printf("%f%%\n", (double)aStats->cpus[i]);
    }
}
