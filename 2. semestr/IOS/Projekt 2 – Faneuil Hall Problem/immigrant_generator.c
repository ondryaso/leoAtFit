#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>

#include "shdata.h"

int immigrant_main(shdata_t *shdata, int id);

int immigrant_generator_main(shdata_t *shdata) {
    pid_t imms[shdata->config.immigrantCount];

    int maxSpawntime = shdata->config.maxImmigrantSpawnTime;

    for (int i = 0; i < shdata->config.immigrantCount; i++) {
        if (maxSpawntime > 0) { // Only wait if IG != 0
            SLEEP_MAX_MS(maxSpawntime);
        }

        imms[i] = fork();

        if (imms[i] == 0) {
            return immigrant_main(shdata, i);
        } else if (imms[i] == -1) {
            // Couldn't generate an immigrant, kill all the living ones and exit
            for (int k = 0; k < i; k++) {
                kill(imms[k], SIGKILL);
            }

            fprintf(stderr, "Couldn't spawn the immigrant #%d. Stopping immigrant generator.\n", i + 1);
            return S_IMMIGRANT_SPAWNING_FAILED;
        }
    }

    int childStatus;
    bool hadBadChild = false;

    for (int i = 0; i < shdata->config.immigrantCount; i++) {
        // Wait for all the immigrants to exit
        waitpid(imms[i], &childStatus, 0);

        if (WIFEXITED(childStatus)) {
            childStatus = WEXITSTATUS(childStatus);

            if (childStatus != 0) {
                fprintf(stderr, "Immigrant #%d exited with status %d.\n", i + 1, childStatus);
                hadBadChild = true;
            }
        } else {
            fprintf(stderr, "Immigrant #%d didn't exit normally.\n", i + 1);
            hadBadChild = true;
        }
    }

    return hadBadChild ? S_IMMIGRANT_ERROR : 0;
}