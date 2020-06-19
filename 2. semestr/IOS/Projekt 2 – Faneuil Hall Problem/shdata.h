// shdata.h
// IOS, projekt 2
// Autor: Ondřej Ondryáš (xondry02, FIT VUT)
// Datum: 19. 4. 2020
// -----------------------------------------
// V tomto souboru je definována struktura držící data sdílená mezi procesy, několik pomocných maker
// a výčet interních chybových kódů.

#ifndef IOS2_SHDATA_H
#define IOS2_SHDATA_H

#include <semaphore.h>
#include <stdbool.h>

#define SLEEP_MAX_MS(ms) usleep(1000 * (rand() % ((ms) + 1)))

// Shortcuts for waiting on the outputMutex semaphore
#define WAIT_OUT() sem_wait(&shdata->outputMutex)
#define POST_OUT() sem_post(&shdata->outputMutex)

enum ErrorStates {
    S_MEMORY_OP_FAILED = -1,
    S_FILE_OPEN_FAILED = -2,
    S_SEMAPHORE_ERROR = -3,
    S_OK = 0,
    S_IMMIGRANT_SPAWNING_FAILED = 1,
    S_IMMIGRANT_ERROR = 2
};

typedef struct config {
    int immigrantCount; // PI
    int maxImmigrantSpawnTime; // IG
    int maxJudgeEntryTime; // JG
    int maxCertPickupTime; // IT
    int maxJudgeConfirmationTime; // JT
} config_t;

typedef struct shdata {
    config_t config;
    FILE *outputFile;

    unsigned int actionCounter;
    unsigned int insideCounter;
    unsigned int checkedInCounter;
    unsigned int enteredWaitingCounter;
    bool judgeInside;

    sem_t outputMutex;
    sem_t checkInMutex;
    sem_t allCheckedIn;
    sem_t noJudge;
    sem_t confirmed;
} shdata_t;

#endif //IOS2_SHDATA_H
