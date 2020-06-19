#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include "shdata.h"

static void print_status(shdata_t *shdata, const char *msg, bool printCounters) {
    shdata->actionCounter++;

    if (printCounters) {
        // enteredWaiting and inside counters are only incremented when the judge is not in the building,
        // and this statement is only called when the judge is inside, so these are protected.
        // The checkedInCounter is always protected by checkInMutex (either locked here or "passed" from an immigrant).
        fprintf(shdata->outputFile, "%u\t: JUDGE\t: %s\t\t: %d\t: %d\t: %d\n",
                shdata->actionCounter, msg, shdata->enteredWaitingCounter, shdata->checkedInCounter,
                shdata->insideCounter);
    } else {
        fprintf(shdata->outputFile, "%u\t: JUDGE\t: %s\n",
                shdata->actionCounter, msg);
    }

    fflush(shdata->outputFile);
}

int judge_main(shdata_t *shdata) {
    int handledImmigrantsCounter = 0;

    while (handledImmigrantsCounter != shdata->config.immigrantCount) {
        SLEEP_MAX_MS(shdata->config.maxJudgeEntryTime);

        WAIT_OUT();
        print_status(shdata, "wants to enter", false);
        POST_OUT();

        // The noJudge semaphore works as a turnstile, it won't let the judge enter
        // when there's currently an immigrant in process of entering
        sem_wait(&shdata->noJudge);
        // Lock the mutex protecting the checkedInCounter
        sem_wait(&shdata->checkInMutex);

        shdata->judgeInside = true;

        WAIT_OUT();
        print_status(shdata, "enters", true);
        POST_OUT();

        if (shdata->enteredWaitingCounter > shdata->checkedInCounter) {
            // If there are immigrants who haven't checked in yet, unlock the
            // check in mutex and wait on allCheckedIn for the last immigrant to signalize
            // they've all checked in
            WAIT_OUT();
            print_status(shdata, "waits for imm", true);
            POST_OUT();

            sem_post(&shdata->checkInMutex);
            sem_wait(&shdata->allCheckedIn);
        }

        WAIT_OUT();
        print_status(shdata, "starts confirmation", true);
        POST_OUT();

        SLEEP_MAX_MS(shdata->config.maxJudgeConfirmationTime);
        int confirmed = (int) shdata->checkedInCounter;

        // Notify all the immigrants waiting on the _confirmed_ semaphore that they can proceed
        WAIT_OUT();
        shdata->checkedInCounter = 0;
        shdata->enteredWaitingCounter = 0;
        print_status(shdata, "ends confirmation", true);
        POST_OUT();

        while (confirmed > 0) {
            sem_post(&shdata->confirmed);
            confirmed--;
            handledImmigrantsCounter++;
        }

        SLEEP_MAX_MS(shdata->config.maxJudgeConfirmationTime);

        WAIT_OUT();
        print_status(shdata, "leaves", true);
        POST_OUT();

        shdata->judgeInside = false;
        sem_post(&shdata->checkInMutex);
        sem_post(&shdata->noJudge);
    }

    return 0;
}