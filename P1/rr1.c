/*
# Family Name: Cakmak

# Given Name: Ege

# Student Number: 215173131

# CSE Login: cakmake
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include "sch-helpers.h"

typedef int bool;
#define true 1
#define false 0

#define numberOfProcessors 4



int processor_1(process *cpu[4]);

int processorAc(process *cpu[4]);

int processorAV(process *cpu[numberOfProcessors]);

int compare(const void *a, const void *b);

int main(int argc, char *argv[]) {

    int quantumTime;

    
    char *temp;
 
    quantumTime= strtol(argv[1], &temp, 10);

   

    process processes[MAX_PROCESSES + 1];
    int numberOfProcesses = 0;
    int numberOfInput;
    int time = 0;
    int contextSwitches = 0;

    // Copies inputted process into the process array.
    while ((numberOfInput = readProcess(&processes[numberOfProcesses]))) {
        if (numberOfInput == 1) {
            numberOfProcesses++;
        }
    }

    // Sorts added processes based on their arrival time.
    qsort(processes, (size_t) numberOfProcesses, sizeof(process), compareByArrival);

    process_queue *queR = malloc(sizeof(queR));
    process_queue *queW = malloc(sizeof(queW));
    initializeProcessQueue(queR);
    initializeProcessQueue(queW);

    /* This is the CPU. We will assign each CPU processes by inserting processes
    into this array. */
    process *cpu[4] = {NULL};
    // Loop invariants.
    int i = 0;
    int j = 0;

    /* An array to store everything before letting them go in the ready queue.
    We will use this array to sort all processes going into that queue based on
    their process IDs.*/
    process *tempqueue[MAX_PROCESSES + 1];
    size_t tempqueueSize = 0;
    // A variable to store total time CPU will be run
    double processingTime = 0;
    while (1) {
        // Inserts arriving processes to tempqueue.
        while (i < numberOfProcesses) {
            if (processes[i].arrivalTime <= time) {
                tempqueue[tempqueueSize++] = &processes[i];
                i++;
            } else {
                break;
            }
        }
        /* Sorts everything in tempqueue by their process ID.
         * tempqueue includes both arriving processes and the ones that are
         * done doing IO.*/
        if (tempqueueSize > 1) {
            qsort(tempqueue, tempqueueSize, sizeof(process * ), compare);
        }

        // Transfers processes in tempqueue to ready queue and sets tempqueueSize to 0.
        for (j = 0; j < tempqueueSize; j++) {
            enqueueProcess(queR, tempqueue[j]);
            tempqueue[j] = NULL;
        }
        tempqueueSize = 0;


        // A variable to store the first available CPU index.
        int freeProcessor;

        // Dispatches processes to available processors.
        while (queR->size > 0 && processorAV(cpu)==1) {
            process_node *aa = queR->front;
            freeProcessor = processor_1(cpu);
            process *current = aa->data;
            current->quantumRemaining = quantumTime;
            cpu[freeProcessor] = current;
            dequeueProcess(queR);
            if (current->currentBurst == 0 &&
                current->bursts[current->currentBurst].step == 0)
                current->startTime = time;
        }

        // Increments waiting time of those processes that are waiting in the ready queue.
        process_node *aa = queR->front;
        for (j = 0; j < queR->size; j++) {
            aa->data->waitingTime++;
            aa = aa->next;
        }

        /* Increments steps and currentsBursts (if needed) of processes that are
         * doing their I/O Bursts.
         * Processes done doing I/O will be put back on a temp array to be sorted
         * then will be put on the ready queue.
         * Processes that are not done will be put back to the end of the queue. */
        int waitingq = queW->size;
        for (j = 0; j < waitingq; j++) {
            process *current = queW->front->data;
            dequeueProcess(queW);
            int currentBurst = current->currentBurst;
            current->bursts[currentBurst].step++;
            if (current->bursts[currentBurst].step == current->bursts[currentBurst].length) {
                current->currentBurst++;
                tempqueue[tempqueueSize] = current;
                tempqueueSize++;

            } else {
                enqueueProcess(queW, current);
            }
        }
        /* Processes the running processes. 
         * The reason we have this after processqueW is to prevent processes
         * from doing their I/O bursts in the same time cycle they did their CPU Bursts.
         * Changing the order does not affect functionality. */
        for (j = 0; j < numberOfProcessors; j++) {
            if (cpu[j] != NULL) {
                processingTime += 100;
                process *current = cpu[j];
                current->quantumRemaining--; // Decrements the time quantum of the process.
                int currentBurst = current->currentBurst;
                if (current->bursts[currentBurst].step < current->bursts[currentBurst].length) {
                    current->bursts[currentBurst].step++;
                }
                if (current->bursts[currentBurst].step == current->bursts[currentBurst].length) {
                    current->currentBurst++;
                    if (current->currentBurst == current->numberOfBursts) {
                        current->endTime = time;
                    } else {
                        enqueueProcess(queW, current);
                    }
                    cpu[j] = NULL;
                    // Preempt the process if its not done within the given time.
                } else if (current->quantumRemaining == 0) {
                    tempqueue[tempqueueSize] = current;
                    contextSwitches++;
                    tempqueueSize++;
                    cpu[j] = NULL;
                }
            }
        }

        // Breaks out of the loop when everything is done.
        if (queW->size == 0 &&
            queR->size == 0 &&
            processorAc(cpu) != 1 &&
            tempqueueSize == 0 &&
            i == numberOfProcesses) {
            break;
        }

        // Increments the CPU time.
        time++;

    }


    double waitingTimeSum = 0;
    for (i = 0; i < numberOfProcesses; i++) {
        waitingTimeSum = waitingTimeSum + processes[i].waitingTime;
    }
    double avgWaitingTime = (double) waitingTimeSum / (double) numberOfProcesses;

    double turnaroundTimeSum = 0;
    for (i = 0; i < numberOfProcesses; i++) {
        int endTime = processes[i].endTime;
        int arrivalTime = processes[i].arrivalTime;
        int turnaroundTime = endTime - arrivalTime + 1;
        turnaroundTimeSum += turnaroundTime;
    }
    double avgTurnaroundTime = (double) turnaroundTimeSum / (double) numberOfProcesses;

    int totalTime = time + 1;
    double averageCPUUtilization = processingTime / (double) totalTime;



    // Gets the most time that was taken by a process.
    int x;
    int maxTime = 0;
    for (x = 0; x < numberOfProcesses; x++) {
        if (processes[x].endTime > maxTime) {
            maxTime = processes[x].endTime;
        }
    }

    printf("Average waiting time                 : %.2f units\n", avgWaitingTime);
    printf("Average turnaround time              : %.2f units\n", avgTurnaroundTime);
    printf("Time all processes finished          : %d\n", totalTime);
    printf("Average CPU utilization              : %.1f %% \n", averageCPUUtilization);
    printf("Number of context switches           : %d\n", contextSwitches);
    printf("PID(s) of last process(es) to finish : ");
    // Prints the last ending pids.
    for (x = 0; x < numberOfProcesses; x++) {
        if (processes[x].endTime == maxTime) {
            printf("%d ", processes[x].pid);
        }
    }
    printf("\n");
    return 0; // Return with no problems.
}






