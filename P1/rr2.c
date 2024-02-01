/*
Family Name:Gandhi   
Given Name:Achal    
Student Number:218319111
EECS Login ID (the one you use to access the red server): achal167
YorkU email address (the one that appears in eClass): achal167@my.yorku.ca
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include "sch-helpers.h"

#define processors 4
int processor_1(process *cpu[4]);
int processorAc(process *cpu[4]);
int processorAV(process *cpu[processors]);
int compare(const void *a, const void *b);

void input(int argc, char *argv[], int *quantum){
    char *temp;
    int a = 0;
    
    if(argc != 2){
        fprintf(stderr,"...");
        exit(1);
    }

    long tempSize = strtol(argv[1],&temp,10);

    if(tempSize > INT_MAX || a!=0 || *temp!='\0'){
        fprintf(stderr,"......");
        exit(1);
    }
    else 
    {
        *quantum = tempSize;
    }

}

int main(int argc, char *argv[]){
    process_queue *queR = malloc(sizeof(queR));
    process_queue *queW = malloc(sizeof(queW));
    int quantumTime;
    input(argc, argv, &quantumTime);
    process processes[MAX_PROCESSES];
    int numberOfProcesses = 0;
    int time = 0;
    int contextSwitches =0;
    int numberOfInput ;
    int processingTime =0;
    double waitingTime_1 = 0;
    double turnaroundTime = 0;
    int availableCPU;
    int i=0;

    int k=0;
    process *tempqueue[MAX_PROCESSES];
    long tempqueueSize = 0;
    while ((numberOfInput = readProcess(&processes[numberOfProcesses]))){
        if(numberOfInput==1){
            numberOfProcesses++;
        }
    }
    qsort(processes,(long) numberOfProcesses, sizeof(process), compareByArrival);

    initializeProcessQueue(queR);
    initializeProcessQueue(queW);
    process *cpu[4] = {NULL};

    while(1){
        
       while (i < numberOfProcesses) {
            if (processes[i].arrivalTime <= time) {
                tempqueue[tempqueueSize++] = &processes[i];
                i++;
            } else {
                break;
            }
        }

            qsort(tempqueue, tempqueueSize, sizeof(process *),compare);

        for (k=0; k<tempqueueSize; k++) {
            enqueueProcess(queR, tempqueue[i]);
            tempqueue[k] = NULL;
        }
        tempqueueSize = 0;

        while (queR->size > 0 && processorAV(cpu)==1){
            process_node *ptr_1 = queR->front;
            availableCPU = processor_1(cpu);
            process *current = ptr_1->data;
            current->quantumRemaining = quantumTime;
            cpu[availableCPU] = current;
            dequeueProcess(queR);
            if (current->currentBurst == 0 &&
                current->bursts[current->currentBurst].step == 0)
                current->startTime = time;
        }

        process_node *ptr_1 = queR->front;
        for(k=0; k < queR->size; k++){
            ptr_1->data->waitingTime++;
            ptr_1 = ptr_1->next;
        }
        
        int waitingq = queW->size;
        for(k=0; k < waitingq; k++){
            process *current= queW->front->data;
            dequeueProcess(queW);
            int currentBurst = current->currentBurst;
            current->bursts[currentBurst].step++;
            if(current->bursts[currentBurst].length == current->bursts[currentBurst].step){
                current->currentBurst++;
                tempqueue[tempqueueSize] = current;
                tempqueueSize++;
            }
            else {
                enqueueProcess(queW,current);
            }
        }

        for(k=0; k < processors; k++){
            if(cpu[k] != NULL){
                processingTime =+ 100;
                process *current = cpu[k];
                current->quantumRemaining--;
                int currentBurst = current->currentBurst;
                if(current->bursts[currentBurst].step < current->bursts[currentBurst].length){
                    current->bursts[currentBurst].step++;
                }
                if(current->bursts[currentBurst].step == current->bursts[currentBurst].length){
                    current->currentBurst++;
                    if(current->currentBurst == current->numberOfBursts){
                        current->endTime = time;
                    }
                    else {
                        enqueueProcess(queW, current);
                    }
                    cpu[k] = NULL;
                }
                else if (current->quantumRemaining == 0){
                    tempqueue[tempqueueSize] = current;
                    contextSwitches++;
                    tempqueueSize++;
                    cpu[k] = NULL;
                }
            }
        }

        if (queW->size == 0 &&
            queR->size == 0 &&
            processorAc(cpu) != 0&&
            tempqueueSize == 0 &&
            i == numberOfProcesses) {
            break;
        }


        time++;

    }

    for(i=0;i<numberOfProcesses;i++){
        waitingTime_1 += processes[i].waitingTime;
    }

    double averagewt = (double) waitingTime_1/(double) numberOfProcesses;

    for(i=0; i<numberOfProcesses; i++){
        int endTime = processes[i].endTime;
        int arrivalTime = processes[i].arrivalTime;
        int a =  endTime - arrivalTime +1;
        turnaroundTime += a;
    }

    double averagett = (double) turnaroundTime/(double)numberOfProcesses;
    int totalTime = time +1;
    double averageUsage = processingTime/(double) totalTime;

    int j = 0;
    int p;
    for (p=0; p<numberOfProcesses; p++){
        if(processes[p].endTime > j){
            j = processes[p].endTime;
        }
    }


    printf("Average waiting time : %.2f units\n"
           "Average turnaround time : %.2f units\n"
           "Time all processes finished : %d\n"
           "Average CPU utilization : %.1f%%\n"
           "Number of context switches : %d\n", 
            averagewt,averagett,totalTime,averageUsage,contextSwitches);
    
    printf("PID(s) of last process(es) to finish :");
    for(i=0;i<numberOfProcesses;i++){
        if(processes[i].endTime == j){
            printf("%d", processes[i].pid);
        }
    }

    printf("\n");


    return 0;


}