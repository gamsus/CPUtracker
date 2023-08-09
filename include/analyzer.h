#ifndef ANALYZER_H
#define ANALYZER_H

#include "proc_stats.h"
#include "analysed_stats.h"

typedef struct Analyzer Analyzer;

Analyzer* analyzerNew(void);

int analazerFunction(Analyzer* analyzer, ProcStats* procStats, AnalysedStats* astats);

void analyzerDelete(Analyzer* analyzer);

#endif
