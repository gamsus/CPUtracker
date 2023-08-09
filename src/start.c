#include <string.h>
#include <sys/time.h>      
#include <stdatomic.h>  
#include <pthread.h>
#include <stdlib.h> 
#include <signal.h>  
#include <stdbool.h>   
#include <time.h>       
#include <stdio.h>   
#include <signal.h>
#include <unistd.h>  
#include <errno.h>

#include "../include/reader.h"
#include "../include/analyzer.h"
#include "../include/printer.h"
#include "../include/buffer.h"
#include "../include/watchdog.h"

int nanosleep(const struct timespec *req, struct timespec *rem);
void signalHandler(int signum);
static void cleaning(void);
static void* readerThreadFunction(void* args);
static void* analyzerThreadFunction(void* args);
static void* printerThreadFunction(void* args);
static void* watchdogThread(void* args);

static Buffer* readerAnalyzerBuffer;
static Buffer* analyzerPrinterBuffer;
static uint8_t cpusNumber;
static volatile sig_atomic_t flag = ATOMIC_VAR_INIT(0);
static atomic_flag watchdogFlag = ATOMIC_FLAG_INIT;
static Analyzer* analyzer;

int start(void) {
  ProcStats stats;
  pthread_t reader_thread;
  pthread_t analyzer_thread;
  pthread_t printer_thread;
  pthread_t watchdogs[3];

  analyzer = analyzerNew();
  reader(&stats);
  cpusNumber = stats.cpuNumber;

  readerAnalyzerBuffer = bufferNew(sizeof(ProcStats) + sizeof(CpuStats) * cpusNumber);
  analyzerPrinterBuffer = bufferNew(sizeof(AnalysedStats) + sizeof(float) * cpusNumber);
  free(stats.oneCpu);

  if(signal(SIGTERM, signalHandler)== SIG_ERR) {
    cleaning();
    return EXIT_FAILURE;
  }
    

  if(signal(SIGINT, signalHandler)== SIG_ERR) {
    cleaning();
    return EXIT_FAILURE;
  }
    

  if(watchdogCreate(&reader_thread, readerThreadFunction, &watchdogs[0], watchdogThread) != 0) {
    cleaning();
    printf("reader join create error");
    return EXIT_FAILURE;
  }

  if(watchdogCreate(&analyzer_thread, analyzerThreadFunction, &watchdogs[1], watchdogThread) != 0) {
    cleaning();
    printf("analyzer join create error");
    return EXIT_FAILURE;
  }

  if(watchdogCreate(&printer_thread, printerThreadFunction, &watchdogs[2], watchdogThread) != 0) {
    cleaning();
    printf("printer join create error");
    return EXIT_FAILURE;
  }

  if(pthread_join(reader_thread, NULL) != 0) {
    printf("reader join error");
    cleaning();
    return EXIT_FAILURE;
  }

  if(pthread_join(analyzer_thread, NULL) != 0) {
    printf("analyzer join error");
    cleaning();
    return EXIT_FAILURE;
  }

  if(pthread_join(printer_thread, NULL) != 0) {
    printf("printer join error");
    cleaning();
    return EXIT_FAILURE;
  }

  for(size_t i = 0; i < 3; i++) {
    if(pthread_join(watchdogs[i], NULL) != 0){
      printf("watchdog error");
      cleaning();
      return EXIT_FAILURE;
    }
  }
  cleaning();
  return 0;
}

void signalHandler(int signum) {
  if(signum == SIGTERM || signum == SIGINT)
    flag = 1;
}

static void* readerThreadFunction(void* args) {
    Signal* signal = (Signal*) args;
    ProcStats stats;
    struct timespec sleepTime;

    while(flag == 0) {   
        reader(&stats);
        if(bufferPush(readerAnalyzerBuffer, &stats, 2) != 1) {
            free(stats.oneCpu);
            pthread_exit(NULL);
        }

        watchdogSendSignal(signal);
        
        sleepTime.tv_sec = 1;
        sleepTime.tv_nsec = 0;
        nanosleep(&sleepTime, NULL);
    }
    pthread_exit(NULL);
    
}

static void* analyzerThreadFunction(void* args) {
    Signal* signal = (Signal*) args;
    ProcStats* stats = malloc(sizeof(ProcStats) + cpusNumber * sizeof(CpuStats));
    AnalysedStats analysed_stats;
    struct timespec sleepTime;

    if(stats == NULL) {
        pthread_exit(NULL);
    }

    while(flag == 0) {
        if(bufferPop(readerAnalyzerBuffer, stats, 2) != 1) {
            free(stats);
            break;
        }
  
        if(analazerFunction(analyzer, stats, &analysed_stats) == 1) {
          if(bufferPush(analyzerPrinterBuffer, &analysed_stats, 2) != 1) {

            free(stats->oneCpu);
            free(analysed_stats.cpus);
            break;
          }
        }

        free(stats->oneCpu);
        watchdogSendSignal(signal);

        sleepTime.tv_sec = 1;
        sleepTime.tv_nsec = 0;
        nanosleep(&sleepTime, NULL);
    }
    free(stats);
    pthread_exit(NULL);
    
}

static void* printerThreadFunction(void* args) {
    Signal* signal = (Signal*) args;
    AnalysedStats* analysed_stats = malloc(sizeof(AnalysedStats) + sizeof(float) * cpusNumber);
    struct timespec sleepTime;

    if (analysed_stats == NULL) {
        pthread_exit(NULL);
    }

    while (flag == 0) {
        if (bufferPop(analyzerPrinterBuffer, analysed_stats, 2) == 1) {
            printer_print_proc_usage(analysed_stats);
            free(analysed_stats->cpus);
            watchdogSendSignal(signal);
            sleepTime.tv_sec = 1;
            sleepTime.tv_nsec = 0;
            nanosleep(&sleepTime, NULL);
        }
    }
    free(analysed_stats);
    pthread_exit(NULL);
    
}

static void* watchdogThread(void* args) {
    Signal* signal = (Signal*)args;

    struct timespec timeout;
    struct timeval now;

    while (flag == 0) {
        gettimeofday(&now, NULL);
        timeout.tv_sec = now.tv_sec + 2;
        timeout.tv_nsec = now.tv_usec * 1000;

        pthread_mutex_lock(&signal->mutex);
        int result = pthread_cond_timedwait(&signal->signal_cv, &signal->mutex, &timeout);
        pthread_mutex_unlock(&signal->mutex);

        if (result == ETIMEDOUT && !flag) {
            perror("Logger no get signal from thread.");

            if (!atomic_flag_test_and_set(&watchdogFlag)) {
                flag = 1;
                break;
            }
        }
    }
    pthread_mutex_destroy(&signal->mutex);
    pthread_cond_destroy(&signal->signal_cv);
    free(signal);
    pthread_exit(NULL);
}

void cleaning(void) {
    ProcStats stats;
    AnalysedStats aStats;
    
    while(!bufferEmpty(readerAnalyzerBuffer)) {
      bufferPop(readerAnalyzerBuffer, &stats, 2);
      free(stats.oneCpu);
    }

    while(!bufferEmpty(analyzerPrinterBuffer)) {
        bufferPop(analyzerPrinterBuffer, &aStats, 2);
        free(aStats.cpus);
    }

    analyzerDelete(analyzer);
    bufferDelete(readerAnalyzerBuffer);
    bufferDelete(analyzerPrinterBuffer);
}

