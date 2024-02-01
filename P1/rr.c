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


#define processors 4 // number of cpus

int processorAV(process *cpu[processors]);
int processor_1(process *cpu[4]);
int processorAc(process *cpu[4]);
int compare(const void *a, const void *b);
int main(int argc, char *argv[]) {
    process_queue *queR = malloc(sizeof(queR));//ready queue
    process_queue *queW = malloc(sizeof(queW));//wwaiting queue
    int quantumTime; //quantum time
    char *temp;   
    process processes[MAX_PROCESSES+1]; // a large array to hold all processes read
    int numberOfProcesses = 0;
    int time = 0; // clock
    int contextSwitches =0; //total number of preemptions
    int numberOfInput ; // number of input 
    int processingTime =0; // total processing time 
    double waitingTime_1 = 0; // total waitingtime
    double turnaroundTime = 0; // total turnaroundtime
    int availableCPU; // available cpu index
    int i,j=0; // loop variants 
    process *tempqueue[MAX_PROCESSES+1]; // temporary queue 
    long tempqueueSize = 0; // temporary queue size
    int p = 0;
    int q = 0;
    
    quantumTime = strtol(argv[1],&temp,10);

    // counting number of processes 
    while ((numberOfInput = readProcess(&processes[numberOfProcesses]))) {
        if (numberOfInput == 1) {
            numberOfProcesses++;
        }
    }

   // sorting processes by which process comes first
    
    qsort(processes,(long) numberOfProcesses, sizeof(process), compareByArrival);
    
    // initializing waiting que and ready to go que
    initializeProcessQueue(queR);
    initializeProcessQueue(queW);

    process *cpu[4] = {NULL};
    
    while (1) {
        /*
        to sort processes and call processes, first storing all processes to temporary queue
        */        
         while (i < numberOfProcesses) {
            if (processes[i].arrivalTime <= time) {
                tempqueue[tempqueueSize++] = &processes[i];
                i++;
            } else {
                break;
            }
        }
       
        /*
            sorting processes in temporary queue based on their processID
        */ 
                
        qsort(tempqueue, tempqueueSize, sizeof(process * ), compare);
        /*
            enqueue processes from temporary queue to ready to go queue and reset the  temporary queue
        */
        for (j = 0; j < tempqueueSize; j++) {
            enqueueProcess(queR, tempqueue[j]);
            tempqueue[j] = NULL;
        }
        tempqueueSize = 0;
        
        /*
        when there are processes in ready to go queue and processors are available 
        processes go to processors.
        */        
        while (queR->size > 0 && processorAV(cpu)==1) {
            process_node *aa = queR->front;
            availableCPU = processor_1(cpu);
            process *current = aa->data;
            current->quantumRemaining = quantumTime;
            cpu[availableCPU] = current;
            dequeueProcess(queR);
            if (current->currentBurst == 0 &&
                current->bursts[current->currentBurst].step == 0)
                current->startTime = time;
        }
        // increament waiting time 
        process_node *aa = queR->front;
        for (j = 0; j < queR->size; j++) {
            aa->data->waitingTime++;
            aa = aa->next;
        }
        /*
        processes that are doing I/O bursts; increament current Bursts and steps
        processes will be transfered to ready to go queue from temporary queue after processes are done with I/O bursts
        */

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
                enqueueProcess(queW, current); // processes will go back to waiting queue if processes arent finished
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
                        enqueueProcess(queW, current);
                    }
                    cpu[j] = NULL;
                } else if (current->quantumRemaining == 0) {
                    tempqueue[tempqueueSize] = current;
                    contextSwitches++;
                    tempqueueSize++;
                    cpu[j] = NULL;
                }
            }
        }

        // reset loop 
        if (queW->size == 0 && queR->size == 0 && processorAc(cpu) != 1 && tempqueueSize == 0 ) {
            break;
        }

        time++;

    }

   
   // increasing waiting time and counting time after each process
    for(i=0;i<numberOfProcesses;i++){
        waitingTime_1 += processes[i].waitingTime;
        int a = processes[i].endTime;
        int b = processes[i].arrivalTime;
        int c = a - b + 1;
        turnaroundTime = turnaroundTime + c;
    }
    //average waiting time and arrival time
    double averagewt = (double) waitingTime_1/ (double) numberOfProcesses; 
    double averagett = (double) turnaroundTime/(double)numberOfProcesses;

    int totalTime = time+1;
    double averageUsage =  processingTime/(double) totalTime;

    printf("Average waiting time : %.2f units\n"
           "Average turnaround time : %.2f units\n"
           "Time all processes finished : %d\n"
           "Average CPU utilization : %.1f%%\n"
           "Number of context switches : %d\n", 
           averagewt,averagett,totalTime,averageUsage,contextSwitches);


    printf("PID(s) of last process(es) to finish :");
    // last process to finish
    for (p=0;p<numberOfProcesses;p++){
        if(processes[p].endTime>q){
            q = processes[p].endTime;
        }
    }
    for (i=0;i<numberOfProcesses;i++){
        if(processes[i].endTime==q){
            printf("%d", processes[i].pid);
        }
    } 
    printf("\n");
    return 0;
}




