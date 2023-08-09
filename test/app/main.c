#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "reader.h"

int main(void) {
    ProcStats stats;
    reader(&stats);
    assert(stats.cpuNumber > 0);
    free(stats.oneCpu);
    return 0;
}