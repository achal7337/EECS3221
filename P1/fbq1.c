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


#define processors 4
void incrementWaitingTimes(process_queue *q1, process_queue *q2, process_queue *q3);
void addfront(process *a, process_queue *b);
int processor_1(process *cpu[4]);
int cpucount(process *cpu[processors]);
int processorAc(process *cpu[4]);
int queuecompare(const void *a, const void *b);
int processorAV(process *cpu[processors]);
int organize(process *cpu[4]);
int compare(const void *a, const void *b);
int processor_2(process *cpuProcesses[4],int a);

void input(int argc,char *argv[],int *a, int *b){
    


    char *temp;
    int e = 0;
   
    long tempsize = strtol(argv[1],&temp,10);

    if(e!=0||tempsize > INT_MAX||*temp!='\0'){
        fprintf(stderr,"....");
        exit(1);
    } else {
        *a = tempsize;
    }
    tempsize = strtol(argv[1],&temp,10);

    if(e!=0||tempsize > INT_MAX||*temp!='\0'){
        fprintf(stderr,"....");
        exit(1);
    } else {
        *b = tempsize;
    }


}

int main(int argc, char *argv[]) {

   
    process_queue *queW = malloc(sizeof(queW));
    process_queue *q0 = malloc(sizeof(q0));
    process_queue *q1 = malloc(sizeof(q1));
    process_queue *q2 = malloc(sizeof(q2));
    int a1,a2;
    input(argc, argv, &a1 ,&a2);
    process processes[MAX_PROCESSES];
    int numberOfProcesses = 0;
    int time = 0;
    int contextSwitches =0;
    int numberOfInput ;
    int processingTime =0;
    double waitingTime_1 = 0;
    double turnaroundTime = 0;
    
    int i=0;
    int j = 0;
    process *tempqueue[MAX_PROCESSES];
    long tempqueueSize = 0;



    while ((numberOfInput = readProcess(&processes[numberOfProcesses]))) {
        if (numberOfInput == 1) {
            numberOfProcesses++;
        }
    }


    
    qsort(processes, (long) numberOfProcesses, sizeof(process), compareByArrival);
    
    initializeProcessQueue(q0);
    initializeProcessQueue(q1);
    initializeProcessQueue(q2);
    initializeProcessQueue(queW);

    process *cpu[4] = {NULL};
    


    while (1) {
        while (i < numberOfProcesses) {
            if (processes[i].arrivalTime <= time) {
                processes[i].currentQueue = 1;
                tempqueue[tempqueueSize++] = &processes[i];
                i++;
            } else {
                break;
            }
        }
        qsort(tempqueue, tempqueueSize, sizeof(process * ), compare);//pidcomparator
        for (j = 0; j < tempqueueSize; j++) {
            if(tempqueue[j]->currentQueue == 1){
                enqueueProcess(q0,tempqueue[j]);
            }
            else if (tempqueue[j]->currentQueue == 2){
                enqueueProcess(q1,tempqueue[j]);
            }
            else if (tempqueue[j]->currentQueue == 3){
                enqueueProcess(q2,tempqueue[j]);
            }
            
            tempqueue[j]= NULL;
        }
        tempqueueSize = 0; 
        
        while (q0->size && processor_2(cpu, 1) == 1) {
           
            int runningProcesses = organize(cpu);

            qsort(cpu, runningProcesses, sizeof(process * ), queuecompare);
            

            
            for (j = 0; j < processors
                        && cpucount(cpu) < q0->size; j++) {
                if (cpu[j] != NULL) {
                    if (cpu[j]->currentQueue == 3) {
                        addfront(cpu[j], q2);
                        cpu[j] = NULL;
                        contextSwitches++;
                    } 
                    else if (cpu[j]->currentQueue == 2) {
                        addfront(cpu[j], q1);
                        cpu[j] = NULL;
                        contextSwitches++;
                    }
                }
            }

            int freeProcessor = processor_1(cpu);
            process *current = q0->front->data;
            if (current->quantumRemaining == 0) {
                current->quantumRemaining = a1;
            }
            if (current->currentBurst == 0 &&
                current->bursts[current->currentBurst].step == 0)
                current->startTime = time;
            cpu[freeProcessor] = current;
            dequeueProcess(q0);
        }


        while(q1->size && processor_2(cpu,2)==1){
            int runningProcesses = organize(cpu);
            if(runningProcesses > 1){
                qsort(cpu, runningProcesses, sizeof(process *), queuecompare);
            }

            for(j = 0; j < processors && cpucount(cpu) < q1->size; j++){
                if(cpu[j] != NULL){
                    if(cpu[j]->currentQueue == 3){
                        addfront(cpu[j],q2);
                        cpu[j]=NULL;
                        contextSwitches++;
                    }
                }
            }
            int freeProcessor = processor_1(cpu);
            process *current = q1->front->data;
            if(current->quantumRemaining == 0){
                current->quantumRemaining = a2;
            }
            if(current->currentBurst==0 && 
                current->bursts[current->currentBurst].step == 0)
                current->startTime = time;
            cpu[freeProcessor] = current;
            dequeueProcess(q1);

        }
      
        while(q2->size && processor_2(cpu,3)==1){
            int freeProcessor = processor_1(cpu);
            process *current = q2->front->data;
            current->quantumRemaining = -1;
            if(current->currentBurst==0 && 
                current->bursts[current->currentBurst].step == 0)
                current->startTime = time;
            cpu[freeProcessor] = current;
            dequeueProcess(q2);
        }  
        incrementWaitingTimes(q0,q1,q2);

       
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
        for (j = 0; j < processors; j++) {
            if (cpu[j] != NULL) {
                processingTime += 100;
                process *current = cpu[j];
                current->quantumRemaining--; 
                int currentBurst = current->currentBurst;
                if (current->bursts[currentBurst].step < current->bursts[currentBurst].length) {
                    current->bursts[currentBurst].step++;
                }
                if (current->bursts[currentBurst].step == current->bursts[currentBurst].length) {
                    current->currentBurst++;
                    if (current->currentBurst == current->numberOfBursts) {
                        current->endTime = time;
                    } else {
                        current->currentQueue = 1;
                        current->quantumRemaining = 0;
                        enqueueProcess(queW, current);
                    }
                    cpu[j] = NULL;
                } else if (current->quantumRemaining == 0) {
                    current->currentQueue++;
                    tempqueue[tempqueueSize] = current;
                    contextSwitches++;
                    tempqueueSize++;
                    cpu[j] = NULL;
                }
            }
        }

        
        if (queW->size == 0 &&
            q0->size == 0 &&
            q1->size == 0 &&
            q2->size == 0 &&
            processorAc(cpu) != 1 &&
            tempqueueSize == 0 &&
            i == numberOfProcesses) {
            break;
        }

        time++;

    }

    
    for (i = 0; i < numberOfProcesses; i++) {
        waitingTime_1 = waitingTime_1 + processes[i].waitingTime;
    }
    double averagewt = (double) waitingTime_1 / (double) numberOfProcesses;
    for (i = 0; i < numberOfProcesses; i++) {
        int endTime = processes[i].endTime;
        int arrivalTime = processes[i].arrivalTime;
        int a =  endTime - arrivalTime +1;
        turnaroundTime += a;
    }
    double averagett = (double) turnaroundTime / (double) numberOfProcesses;

    int totalTime = time + 1;
    double averageUsage = processingTime / (double) totalTime;



    int x;
    int maxTime = 0;
    for (x = 0; x < numberOfProcesses; x++) {
        if (processes[x].endTime > maxTime) {
            maxTime = processes[x].endTime;
        }
    }

    printf("Average waiting time : %.2f units\n"
           "Average turnaround time : %.2f units\n"
           "Time all processes finished : %d\n"
           "Average CPU utilization : %.1f%%\n"
           "Number of context switches : %d\n", 
           averagewt,averagett,totalTime,averageUsage,contextSwitches);


    printf("last process to finish:");
    for (x = 0; x < numberOfProcesses; x++) {
        if (processes[x].endTime == maxTime) {
            printf("%d ", processes[x].pid);
        }
    }
    printf("\n");


    return 0; 
}
