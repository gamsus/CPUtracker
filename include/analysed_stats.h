#ifndef ANALYSED_PROC_STATISTICS_H
#define ANALYSED_PROC_STATISTICS_H

#include <stdint.h> // uint8_t

typedef struct AnalysedStats {
  uint8_t cpus_number;
  float* cpus;
  float total;
  char padding[3];
} AnalysedStats;

#endif // ANALYSED_PROC_STATISTICS_H
