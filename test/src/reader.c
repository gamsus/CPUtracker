#include "reader.h"

#include <stdio.h> 
#include <stdlib.h>     
#include <sys/stat.h>   
#include <stdbool.h>
#include "string.h" 

int reader(ProcStats* stats) {
  uint8_t cpusNumber = 0;
  size_t cpuNum = 0;
  int helper;
  FILE *file4 = fopen("/proc/stat", "r");
  FILE *file3 = fopen("/proc/stat", "r");
  char line1[1024];
  char line[1024];

  if(stats == NULL) {
    return 0;
  }
  
  while (!(fgets(line, sizeof(line), file4) == NULL))
  {
    if(strstr(line, "cpu") != NULL) {
      cpusNumber++;
    }
  }
  
  cpusNumber--;
  stats->cpuNumber = cpusNumber;
  stats->oneCpu = malloc(sizeof(CpuStats) * cpusNumber);

  if(stats->oneCpu == NULL) {
    return 0;
  }

  while (!(fgets(line1, sizeof(line1), file3) == NULL)) {
    if(strstr(line1, "cpu") != NULL) {
      if(strstr(line1, "cpu") != NULL) {
        if(cpuNum == 0) {
          sscanf(line1, "cpu %d %d %d %d %d %d %d %d", &(stats->total.user), &(stats->total.nice),
          &(stats->total.system), &(stats->total.idle), &(stats->total.iowait),
          &(stats->total.irq), &(stats->total.sortirq), &(stats->total.steal));
          }
        else {
          sscanf(line1, "cpu%d %d %d %d %d %d %d %d %d", &helper, &(stats->oneCpu[cpuNum - 1].user), &(stats->oneCpu[cpuNum - 1].nice),
          &(stats->oneCpu[cpuNum - 1].system), &(stats->oneCpu[cpuNum - 1].idle), &(stats->oneCpu[cpuNum - 1].iowait),
          &(stats->oneCpu[cpuNum - 1].irq), &(stats->oneCpu[cpuNum - 1].sortirq), &(stats->oneCpu[cpuNum - 1].steal));
        }
        cpuNum++;
      }
    }
  }

  fclose(file3);
  fclose(file4);
  return 1;
}