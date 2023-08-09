#ifndef PROC_STATS_H
#define PROC_STATS_H

#include <stdint.h>

typedef struct CpuStats {
  uint32_t user;
  uint32_t nice;
  uint32_t system;
  uint32_t idle;
  uint32_t iowait;
  uint32_t irq;
  uint32_t sortirq;
  uint32_t steal;
} CpuStats;

typedef struct ProcStats {
  CpuStats total;
  CpuStats* oneCpu;
  uint8_t cpuNumber;
  char padding[7];
} ProcStats;

#endif
