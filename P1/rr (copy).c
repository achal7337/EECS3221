/*
Family Name:Gandhi   
Given Name:Achal    
Student Number:218319111
EECS Login ID (the one you use to access the red server): achal167
YorkU email address (the one that appears in eClass): achal167@my.yorku.ca
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>
#include <errno.h>
#include <limits.h>
#include "sch-helpers.h"
#define processors 4
int processor_1(process *cpuProcesses[processors]);
int processorAV(process *cpuProcesses[processors]);
int processorAc(process *cpuProcesses[processors]);
int compare(const void *aa, const void*bb);

void input(int argc,char *argv[],int *quantum){
    if(argc < 2 || argc > 2){
        fprintf(stderr,"...");
        exit(1);
    }


    char *temp;
    int a = 0;
    
    long tempsize = strtol(argv[1],&temp,10);

    if(a!=0||tempsize > INT_MAX||*temp!='\0'){
        fprintf(stderr,"....");
        exit(1);
    } else {
        *quantum = tempsize;
    }

    if(*quantum < 1|| *quantum >(powf(2,31)-1)){
        fprintf(stderr,"..");
        exit(1);
    }


}
void printResults(process processes[], double totalCPUTime, int clock, int contextSwitches, int numberOfProcesses) {
    int i;
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

    int totalTime = clock + 1;
    double averageCPUUtilization = totalCPUTime / (double) totalTime;



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

}


int main(int argc,char *argv[]){
    int quantumTime;
    input(argc,argv,&quantumTime);
    process processes[MAX_PROCESSES+1];
    int np = 0;
    int cs = 0;
    int clock = 0;
    int st ;
    double tt=0;

    while((st  = readProcess(&processes[np]))){
        if(st == 1){
            np++;
        }
    }

    qsort(processes,(size_t)np,sizeof(process),compareByArrival);

    process_queue *queR = malloc(sizeof(queR));
    process_queue *queW = malloc(sizeof(queW));
    initializeProcessQueue(queR);
    initializeProcessQueue(queW);


    process *cpuProcesses[4]={NULL};

    int i,j;
    process *incomingProcessque[MAX_PROCESSES+1];
    size_t incomingProcessquesize = 0;
    while(1){
        i=0;
        while (i<np){
            if(processes[i].arrivalTime<=clock) {
                incomingProcessque[incomingProcessquesize++]=&processes[i];
                i++;
            }else {
                break;
            }
        }

        if(incomingProcessquesize>1){
            qsort(incomingProcessque,incomingProcessquesize,sizeof(process *),compare);
        }

        for (j=0;j<incomingProcessquesize;j++){
            enqueueProcess(queR,incomingProcessque[j]);
            incomingProcessque[j]=NULL;
        }
        incomingProcessquesize = 0;

        int nextCpu;

        while(queR->size > 0 && processorAV(cpuProcesses)==1){
            process_node *aa = queR-> front;
            nextCpu = processor_1(cpuProcesses);
            process *current = aa->data;
            current->quantumRemaining = quantumTime;
            cpuProcesses[nextCpu] = current;
            dequeueProcess(queR);
            if(current-> currentBurst == 0 && current -> bursts[current->currentBurst].step==0)
                {current->startTime = clock;}
        }

        process_node *aa = queR->front;
        for(j=0;j < queR->size;j++){
            aa->data->waitingTime++;
            aa = aa->next;
        }

        int queWsize;
        queWsize = queW -> size;
        for(j=0;j<queWsize;j++){
            process *current = queW->front->data;
            dequeueProcess(queW);
            int currentBurst = current->currentBurst;
            current->bursts[currentBurst].step++;
            if(current->bursts[currentBurst].step == current->bursts[currentBurst].length){
                current->currentBurst++;
                incomingProcessque[incomingProcessquesize]=current;
                incomingProcessquesize++;
            }else {
                enqueueProcess(queW,current);
            }
        }

        for(j=0;j<np;j++){
            if(cpuProcesses[j]!=NULL){
                tt += 100;
                process *current = cpuProcesses[j];
                current -> quantumRemaining--;
                int currentBurst = current->currentBurst;
                if(current->bursts[currentBurst].step<current->bursts[currentBurst].length){
                    current->currentBurst++;
                }
                if(current->bursts[currentBurst].step==current->bursts[currentBurst].length){
                    current -> currentBurst++;
                    if(current->currentBurst == current->numberOfBursts){
                        current->endTime = clock;
                    }else{
                        enqueueProcess(queW,current);
                    }
                    cpuProcesses[j]=NULL;
                }else if(current->quantumRemaining==0){
                    incomingProcessque[incomingProcessquesize]= current;
                    cs++;
                    incomingProcessquesize++;
                    cpuProcesses[j]=NULL;
                }
            }
        }

        if(queW->size==0 && queR->size==0 && processorAc(cpuProcesses)!=1 && incomingProcessquesize==0 && i == np){
            break;
        }

        clock++;




    }

    /*double waitingTime = 0;
    double turnt = 0;
    for(i=0;i<np;i++){
        waitingTime += processes[i].waitingTime;

    }
    double awt = (double) waitingTime/ (double) np;
    for (i=0;i<np;i++){
        int endTime = processes[i].endTime;
        int arrivalTime = processes[i].arrivalTime;
        int tt1 =  endTime - arrivalTime +1;
        turnt += tt1;

    }

    int avgtt = (double) turnt/(double)np;

    int totalt = clock+1;
    double avgcpu =  tt/(double) totalt;


    int p;
    int max = 0;
    for (p=0;p<np;p++){
        if(processes[p].endTime>max){
            max = processes[p].endTime;
        }
    }

    printf("Average waiting time : %.2d units\n"
           "Average turnaround time : %.2f units\n"
           "Time all processes finished : %d\n"
           "Average CPU utilization : %.1f%%\n"
           "Number of context switches : %d\n", 
           avgtt,awt,totalt,avgcpu,cs);


    printf("last process to finish:");


    for (i=0;i<np;i++){
        if(processes[i].endTime==max){
            printf("%d", processes[i].pid);
        }
    }
    
    printf("\n");

    return 0;
*/
}
