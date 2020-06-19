// main.c
// IOS, projekt 2
// Autor: Ondřej Ondryáš (xondry02, FIT VUT)
// Datum: 19. 4. 2020
// -----------------------------------------
// V tomto souboru je definován hlavní vstupní bod programu a všechny pomocné funkce, které inicializují program
// do potřebného stavu (naparsují argumenty, vytvoří sdílenou paměť, semafory, otevřou výstupní soubor) a vytvoří
// požadované procesy, jejichž funkcionalita je definována v dalších modulech.
// -----------------------------------------
// Program končí kódem 0, pokud vše proběhlo v pořádku, nebo kódem, který je složen z bit flags definovaných
// v makrech ERR_* podle chyb, které nastaly. Chybová hlášení jsou vypisována na stderr, výstup programu je uložen
// do souboru proj2.out (změnou makra OUTPUT_TO_FILE na 0 se výstup přepne na stdout).
// -----------------------------------------
// Program využívá nepojmenovaných semaforů, které jsou uloženy v datové struktuře shdata. Řešení vychází z příkladu
// v The Little Book of Semaphores, oproti ní přidává zejména semafor outputMutex, který slouží pro dosažení vyloučení
// v přístupu k výstupnímu souboru, ale také zamyká přístup ke sdíleným proměnným enteredWaitingCounter,
// checkedInCounter a insideCounter, které jsou při vypisování výstupu čteny a pokud není přístup k nim uzamknut,
// čas od času se stane, že hodnoty nejsou úplně konzistentní (myslím si, že tento problém nastal i ve vzorovém výstupu,
// konkrétně v souboru proj2.out.3, kde na řádcích 322 a 326 vypisuje program NC=NE=0, i když před tímto výstupem už
// do budovy stihl imigrant vstoupit). To je také důvod, proč se na semaforu outputMutex nečeká až ve vypisovacích
// funkcích, ale zamyká/odemyká se makry WAIT_OUT() a POST_OUT(), kterými obaluji nejen volání funkce, ale právě
// i přístupy k příslušným proměnným.

#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <semaphore.h>
#include <limits.h>
#include <time.h>

#include "shdata.h"
#include "string.h"

// Error flag used when the arguments are invalid
#define ERR_ARGS 1u
// Error flag used when a memory allocation/deallocation problem occurs
#define ERR_MEM 2u
// Error flag used when an IO or semaphore error occurs
#define ERR_SYS 4u
// Error flag used when the main process fails to spawn one of its child process
#define ERR_SPAWN 8u
// Error flag used when one of the child processes ends in an abnormal way
#define ERR_PROC 16u

#define OUTPUT_TO_FILE 1
#define OUTPUT_FILE_NAME "proj2.out"

int immigrant_generator_main(shdata_t *shdata);

int judge_main(shdata_t *shdata);

// Closes all the semaphores.
// If a sem_close() call ends with an error, it is quietly discarded.
int close_semaphores(shdata_t *shdata) {
    bool hadError = false;

    if (sem_destroy(&shdata->outputMutex) == -1)
        hadError = true;
    if (sem_destroy(&shdata->checkInMutex) == -1)
        hadError = true;
    if (sem_destroy(&shdata->allCheckedIn) == -1)
        hadError = true;
    if (sem_destroy(&shdata->noJudge) == -1)
        hadError = true;
    if (sem_destroy(&shdata->confirmed) == -1)
        hadError = true;

    return hadError ? S_SEMAPHORE_ERROR : S_OK;
}

// A macro that initialises the specified semaphore in shdata_t and checks for an error.
// If an error occurs, prints it and calls return.
#define SEM_INIT_CHECK(name, val) if (sem_init(&shdata->name, 1, (val)) == -1) \
    { perror("Couldn't initialise a semaphore"); return S_SEMAPHORE_ERROR; }

// Creates unnamed semaphores stored in the specified shdata_t structure.
int init_semaphores(shdata_t *shdata) {
    SEM_INIT_CHECK(outputMutex, 1);
    SEM_INIT_CHECK(checkInMutex, 1);
    SEM_INIT_CHECK(noJudge, 1);
    SEM_INIT_CHECK(confirmed, 0);
    SEM_INIT_CHECK(allCheckedIn, 0);

    return S_OK;
}

// Unmaps/deallocates the memory allocated by prepare_share_data().
int close_memory(shdata_t *mem) {
    if (munmap(mem, sizeof(shdata_t)) == -1) {
        perror("Couldn't unmap shared memory");
        return S_MEMORY_OP_FAILED;
    }

    return S_OK;
}

// Creates the shared data structure:
// > Creates a mapping in the virtual memory using mmap() that is ANONYMOUS (not backed by a file) and SHARED,
//   so it can be passed to the children processes and they can access it.
// > Clears the allocated memory (memset to zero).
// > Calls init_semaphores().
// > Opens the output file and stores its descriptor in the structure.
// If any of those actions fails, cleans up and returns an error code. Otherwise, stores a pointer
// to the created structure in *target and returns S_OK.
int prepare_shared_data(shdata_t **target) {
    int prot = PROT_READ | PROT_WRITE;
    int flags = MAP_SHARED | MAP_ANONYMOUS;
    size_t s = sizeof(shdata_t);

    shdata_t *mem = mmap(NULL, s, prot, flags, -1, 0);

    if (mem == MAP_FAILED) {
        perror("Couldn't allocate shared memory");
        return S_MEMORY_OP_FAILED;
    }

    memset(mem, 0, sizeof(shdata_t));
    if (init_semaphores(mem) != S_OK) {
        close_memory(mem);
        return S_SEMAPHORE_ERROR;
    }

#if OUTPUT_TO_FILE
    mem->outputFile = fopen(OUTPUT_FILE_NAME, "w+");
    if (mem->outputFile == NULL) {
        perror("Couldn't open the output file");
        close_memory(mem);
        close_semaphores(mem);
        return S_FILE_OPEN_FAILED;
    }
#else
    mem->outputFile = stdout;
#endif

    *target = mem;
    return S_OK;
}

// A macro used in parse_args() that parses a number from the i-th entry of the argv array,
// checks whether the whole string was a number and whether the loaded number is in bounds.
// Stores the loaded number in the specified member of the config_t structure.
#define LOAD_ARG(name, i, lower, upper) loaded = strtol(argv[i], &end, 10); \
    if (loaded < (lower) || loaded > (upper)) { *success = 0; } \
    if (*end != '\0') { *success = 0; } \
    ret.name = loaded;

config_t parse_args(char **argv, int *success) {
    config_t ret;
    char *end;
    int loaded;
    *success = 1;

    LOAD_ARG(immigrantCount, 1, 1, INT_MAX);
    LOAD_ARG(maxImmigrantSpawnTime, 2, 0, 2000);
    LOAD_ARG(maxJudgeEntryTime, 3, 0, 2000);
    LOAD_ARG(maxCertPickupTime, 4, 0, 2000);
    LOAD_ARG(maxJudgeConfirmationTime, 5, 0, 2000);

    return ret;
}

unsigned int cleanup(shdata_t *shdata) {
    unsigned int retCode = 0;

    if (close_semaphores(shdata) != S_OK) {
        retCode |= ERR_SYS;
    }

    if (fclose(shdata->outputFile) != 0) {
        retCode |= ERR_SYS;
    }

    if (close_memory(shdata) == S_MEMORY_OP_FAILED) {
        retCode |= ERR_MEM;
    }

    return retCode;
}

// Waits for the children processes, checks the way they exited and return a number
// composed of the corresponding ERR_ flags.
unsigned int wait_for_children(pid_t judgePid, pid_t immSpawnerPid) {
    int childStatus = 0;
    unsigned int retCode = 0;

    // The immigrant spawner must be waited for first, because if it fails, it needs to kill the judge.

    waitpid(immSpawnerPid, &childStatus, 0);
    if (WIFEXITED(childStatus)) {
        childStatus = WEXITSTATUS(childStatus);

        if (childStatus != 0) {
            fprintf(stderr, "The immigrant spawner ended with code %d.\n", childStatus);
            retCode |= ERR_PROC;

            if (childStatus == S_IMMIGRANT_SPAWNING_FAILED) {
                retCode |= ERR_SPAWN;
            }

            // Send a kill signal to the judge as he would keep entering the building and leaving,
            // because his terminating condition (confirmed == PI) will not be fulfilled now.
            kill(judgePid, SIGKILL);
        }
    } else {
        fprintf(stderr, "The immigrant spawner didn't exit normally.\n");
        retCode |= ERR_PROC;
        kill(judgePid, SIGKILL);
    }

    waitpid(judgePid, &childStatus, 0);
    if (WIFEXITED(childStatus)) {
        childStatus = WEXITSTATUS(childStatus);

        if (childStatus != 0) {
            fprintf(stderr, "The judge process ended with code %d.\n", childStatus);
            retCode |= ERR_PROC;
        }
    } else {
        fprintf(stderr, "The judge process didn't exit normally.\n");
        retCode |= ERR_PROC;
    }

    return retCode;
}

int main(int argc, char **argv) {
    if (argc != 6) {
        fprintf(stderr, "Invalid argument count.\nCorrect usage: %s PI IG JG IT JT\n", argv[0]);
        return ERR_ARGS;
    }

    int argParseRes;
    config_t config = parse_args(argv, &argParseRes);
    if (!argParseRes) {
        fprintf(stderr,
                "PI must be an integer >= 1. The other arguments must be integers >= 0 and <= 2000.\nCorrect usage: %s PI IG JG IT JT\n",
                argv[0]);
        return ERR_ARGS;
    }

    shdata_t *shdata;
    int prepRes = prepare_shared_data(&shdata);

    if (prepRes == S_MEMORY_OP_FAILED) {
        return ERR_MEM;
    } else if (prepRes != S_OK) {
        return ERR_SYS;
    }

    shdata->config = config;

    // Seed the rand() function with current time.
    srand(time(NULL));

    pid_t judgePid = fork();
    if (judgePid == 0) {
        // A wild judge has been spawned. Jump to his code.
        return judge_main(shdata);
    } else if (judgePid == -1) {
        // Couldn't spawn the judge
        fprintf(stderr, "Couldn't spawn the judge process.\n");
        cleanup(shdata);
        return ERR_SPAWN;
    }

    pid_t immSpawnerPid = fork();
    if (immSpawnerPid == 0) {
        // The immigrant generator has been spawned.
        return immigrant_generator_main(shdata);
    } else if (immSpawnerPid == -1) {
        // Couldn't spawn the immigrant spawner, kill the judge and leave.
        fprintf(stderr, "Couldn't spawn the immigrant generator process.\n");
        kill(judgePid, SIGKILL);
        cleanup(shdata);
        return ERR_SPAWN;
    }

    // Wait for the children to end and collect the result of their execution
    unsigned int retCode = wait_for_children(judgePid, immSpawnerPid);

    // Cleanup
    retCode |= cleanup(shdata);

    return (int) retCode;
}
