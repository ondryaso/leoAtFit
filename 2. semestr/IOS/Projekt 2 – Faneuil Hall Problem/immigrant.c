#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

#include "shdata.h"

static void print_status(shdata_t *shdata, int id, const char *msg, bool printCounters) {
    shdata->actionCounter++;

    if (printCounters) {
        fprintf(shdata->outputFile, "%u\t: IMM %d\t: %s\t\t: %d\t: %d\t: %d\n",
                shdata->actionCounter, id + 1, msg, shdata->enteredWaitingCounter, shdata->checkedInCounter,
                shdata->insideCounter);
    } else {
        fprintf(shdata->outputFile, "%u\t: IMM %d\t: %s\n",
                shdata->actionCounter, id + 1, msg);
    }

    fflush(shdata->outputFile);
}


int immigrant_main(shdata_t *shdata, int id) {
    WAIT_OUT();
    print_status(shdata, id, "starts", false);
    POST_OUT();

    // Wait until there's no judge
    sem_wait(&shdata->noJudge);

    // Enter the building
    WAIT_OUT();
    shdata->enteredWaitingCounter++;
    shdata->insideCounter++;
    print_status(shdata, id, "enters", true);
    POST_OUT();

    // The judge and anyone else may now enter
    sem_post(&shdata->noJudge);
    sem_wait(&shdata->checkInMutex); // lock for checked in counter

    WAIT_OUT();
    shdata->checkedInCounter++;
    print_status(shdata, id, "checks", true);
    POST_OUT();

    if (shdata->judgeInside && shdata->checkedInCounter == shdata->enteredWaitingCounter) {
        // If the judge is inside the building now and all the immigrants inside have checked in,
        // synchronize on allCheckedIn and let the judge do his thing. He'll take care of the mutex,
        // no one else may get checked in at this point anyway - there will me no new immigrants inside
        // as noJudge is locked when there's the judge inside.
        sem_post(&shdata->allCheckedIn);
    } else {
        sem_post(&shdata->checkInMutex); // unlock the checked in counter
    }

    // Wait for the judge to confirm all the immigrants
    sem_wait(&shdata->confirmed);

    WAIT_OUT();
    print_status(shdata, id, "wants certificate", true);
    POST_OUT();

    SLEEP_MAX_MS(shdata->config.maxCertPickupTime);

    WAIT_OUT();
    print_status(shdata, id, "got certificate", true);
    POST_OUT();

    // Wait until there's no judge
    sem_wait(&shdata->noJudge);

    WAIT_OUT();
    shdata->insideCounter--;
    print_status(shdata, id, "leaves", true);
    POST_OUT();

    sem_post(&shdata->noJudge);

    return 0;
}