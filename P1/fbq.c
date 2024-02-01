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
// declaration of methods
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
   
    process_queue *queW = malloc(sizeof(queW)); // waiting queue
    process_queue *q0 = malloc(sizeof(q0)); //queue 0
    process_queue *q1 = malloc(sizeof(q1)); //queue 1
    process_queue *q2 = malloc(sizeof(q2)); //queue 2
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
    int i,j=0;
    process *tempqueue[MAX_PROCESSES];
    long tempqueueSize = 0;

    //counting number of process
    while ((numberOfInput = readProcess(&processes[numberOfProcesses]))) {
        if (numberOfInput == 1) {
            numberOfProcesses++;
        }
    }

   //  sorting processes by which process comes first
    qsort(processes,(long) numberOfProcesses, sizeof(process), compareByArrival);

    // initializing queues
    initializeProcessQueue(queW);
    initializeProcessQueue(q0);
    initializeProcessQueue(q1);
    initializeProcessQueue(q2);

    process *cpu[4] = {NULL};
    
   
   
   
    while (1) {
        /*
        to sort processes and call processes, first storing all processes to temporary queue
        */
        while (i < numberOfProcesses) {
            if (processes[i].arrivalTime <= time) {
                processes[i].currentQueue = 1;
                tempqueue[tempqueueSize++] = &processes[i];
                i++;
            } else {
                break;
            }
        }
        /*
            sorting processes in temporary queue based on their processID
        */ 
        qsort(tempqueue, tempqueueSize, sizeof(process * ), compare);//pidcomparator
        
        /*
            enqueue processes from temporary queue to q0,q1,q2 and reset the  temporary queue
        */
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

         /*
        when there are processes in q1 queue and processors are available 
        processes go to processor.
        */
        while (q0->size && processor_2(cpu, 1) == 1){
            
            int runningProcesses = organize(cpu);

            // sort array by comparing queues higher level queues comes before and same level queues compares by thier process id
            qsort(cpu,runningProcesses,sizeof(process *), queuecompare);
            
            // free processors and add processes in front of their queue
            for(j=0; j < processors && cpucount(cpu) < q0->size; j++){
                if(cpu[j] != NULL){
                    if(cpu[j]->currentQueue == 3){
                        addfront(cpu[j],q2);
                        cpu[j]=NULL;
                        contextSwitches++;
                    }
                    else if (cpu[j]->currentQueue == 2){
                        addfront(cpu[j],q1);
                        cpu[j] = NULL;
                        contextSwitches++;
                    }
                }
            }


            int freeProcessor = processor_1(cpu);
            process *current = q0->front->data;
            if(current->quantumRemaining == 0){
                current->quantumRemaining = a1;
            }
            if(current->currentBurst == 0 && 
                current->bursts[current->currentBurst].step == 0)
                current->startTime = time;
            cpu[freeProcessor] = current;
            dequeueProcess(q0);

        }

        // transfers processes in queue 0

        while(q1->size && processor_2(cpu,2)==1){
            int runningProcesses = organize(cpu);
           
            qsort(cpu, runningProcesses, sizeof(process *), queuecompare);
            

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
            if(current->currentBurst==0 && current->bursts[current->currentBurst].step == 0)
                current->startTime = time;
            cpu[freeProcessor] = current;
            dequeueProcess(q1);

        }       
        // transfers processes in queue 1
        while(q2->size && processor_2(cpu,3)==1){
            int freeProcessor = processor_1(cpu);
            process *current = q2->front->data;
            current->quantumRemaining = -1;
            if(current->currentBurst==0 && current->bursts[current->currentBurst].step == 0)
                current->startTime = time;
            cpu[freeProcessor] = current;
            dequeueProcess(q2);
        }  
        // increament waiting time of queue
        process_node *b0 = q0->front;
        for (j = 0; j< q0->size; j++) {
            b0->data->waitingTime++;
            b0 = b0->next;
        }
        process_node *b1 = q1->front;
        for (j = 0; j < q1->size; j++) {
            b1->data->waitingTime++;
            b1 = b1->next;
        }
        process_node *b2 = q2->front;
        for (j = 0; j < q2->size; j++) {
            b2->data->waitingTime++;
            b2 = b2->next;
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

        // reset loop
        if (queW->size == 0 && q0->size == 0 && q1->size == 0 && q2->size == 0 && processorAc(cpu) != 1 && tempqueueSize == 0 ) {
            break;
        }

        time++;

    }

   
   
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


    int p;
    int max = 0;
    for (p=0;p<numberOfProcesses;p++){
        if(processes[p].endTime>max){
            max = processes[p].endTime;
        }
    }

    printf("Average waiting time : %.2f units\n"
           "Average turnaround time : %.2f units\n"
           "Time all processes finished : %d\n"
           "Average CPU utilization : %.1f%%\n"
           "Number of context switches : %d\n", 
           averagewt,averagett,totalTime,averageUsage,contextSwitches);


    printf("last process to finish:");


    for (p=0;p<numberOfProcesses;p++){
        if(processes[p].endTime==max){
            printf("%d", processes[p].pid);
        }
    }
    
    printf("\n");

    
    return 0;
}

