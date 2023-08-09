#include <stdlib.h> 
#include <stdbool.h>   
#include <stdio.h>     

#include "../include/analyzer.h"

static float analyze(CpuStats *statistics, uint64_t *totalPrev, uint64_t *idlePrev);

typedef struct Analyzer {
  uint64_t* cpusTotalPrev;
  uint64_t* cpusIdlePrev;
  uint64_t procTotalPrev;
  uint64_t procIdlePrev;
  bool firstInit;
  char padding[7];
} Analyzer;

Analyzer* analyzerNew(void) {
    Analyzer* analyzer = malloc(sizeof(*analyzer));
    if(analyzer == NULL) {
      return NULL;
    }
    
    analyzer->firstInit = false;
    analyzer->cpusTotalPrev = NULL;
    analyzer->cpusIdlePrev = NULL;
    return analyzer;
}

static float analyze(CpuStats *statistics, uint64_t *totalPrev, uint64_t *idlePrev) {
  float percentage = 0.0f;
  uint64_t idle = statistics->idle + statistics->iowait;
  uint64_t nonIdle = statistics->user + statistics->nice + statistics->system + statistics->irq + statistics->sortirq + statistics->steal;
  uint64_t total = idle + nonIdle - *totalPrev;
  uint64_t idled = idle - *idlePrev;
  if(total != 0) {
    percentage = (float)(total - idled) / (float)total * 100.0f;
  }
  *totalPrev = idle + nonIdle;
  *idlePrev = idle;
  return percentage;
}

int analazerFunction(Analyzer *analyzer, ProcStats *procStats, AnalysedStats *astats) {
  uint64_t idle;
  uint64_t non_idle;

  if(analyzer == NULL) {
    return 0;
  }
  if(procStats == NULL) {
    return 0;
  }
  if(astats == NULL) {
    return 0;
  }
    
  if(!analyzer->firstInit) {
    analyzer->cpusTotalPrev = malloc(sizeof(uint64_t) * procStats->cpuNumber);
    if(analyzer->cpusTotalPrev == NULL) {
      return 0;
    }
    analyzer->cpusIdlePrev = malloc(sizeof(uint64_t) * procStats->cpuNumber);
    if(analyzer->cpusIdlePrev == NULL) {
      free(analyzer->cpusTotalPrev);
      return 0;
    }
    idle = procStats->total.idle + procStats->total.iowait;
    non_idle = procStats->total.user + procStats->total.nice + procStats->total.system + procStats->total.irq + procStats->total.sortirq + procStats->total.steal;
    analyzer->procTotalPrev = idle + non_idle;
    analyzer->procIdlePrev = idle;
    
    for(uint64_t i = 0; i < procStats->cpuNumber; i++) {
      idle = procStats->oneCpu[i].idle + procStats->oneCpu[i].iowait;
      non_idle = procStats->oneCpu[i].user + procStats->oneCpu[i].nice + procStats->oneCpu[i].system + procStats->oneCpu[i].irq +procStats->oneCpu[i].sortirq + procStats->oneCpu[i].steal;
    
      analyzer->cpusTotalPrev[i] = idle + non_idle;
      analyzer->cpusIdlePrev[i] = idle;
    }
    analyzer->firstInit = true;
    astats->cpus = NULL;
    astats->cpus_number = procStats->cpuNumber;
    return 0;
  }

  astats->cpus = malloc(sizeof(float) * procStats->cpuNumber);
  if(astats->cpus == NULL) {
    return 0;
  }
    
  astats->cpus_number = procStats->cpuNumber;
  astats->total = analyze(&procStats->total, &analyzer->procTotalPrev, &analyzer->procIdlePrev);
  for(uint64_t i = 0; i < procStats->cpuNumber; i++) {
    astats->cpus[i] = analyze(&(procStats->oneCpu[i]), &(analyzer->cpusTotalPrev[i]), &(analyzer->cpusIdlePrev[i]));
  }
  return 1;
}

void analyzerDelete(Analyzer* analyzer) {
  if(analyzer == NULL) {
    return;
  }
  if(analyzer->cpusTotalPrev != NULL) {
    free(analyzer->cpusTotalPrev);
  }  
  if(analyzer->cpusIdlePrev != NULL) {
    free(analyzer->cpusIdlePrev);
  }
  free(analyzer);
}
