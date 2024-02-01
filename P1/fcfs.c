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
#include "sch-helpers.h"

int i,j,k;
int np ; // number of process
int tt ; //simulation time
int bt ; //burst time cpu time
int wt ; // wait time
int cswitch; // total context switch
int next; // next process
int t;
process *temp[MAX_PROCESSES+1]; // sorting purposing temp que
int tempq;// temporary que size

process_queue queR; // ready queue to store processes
process_queue queW; //waiting queue to store processes which are in waiting
process processes[MAX_PROCESSES+1]; // array to store all process
process *cpu[NUMBER_OF_PROCESSORS]; //processes running on each cpu



// returns how many processes are currently executing
int current(void){
    int a = 0;
    for (i=0;i<NUMBER_OF_PROCESSORS;i++){
        if(cpu[i]!=NULL)
            a++;
        
    }
    return a;
}
int incoming(void){
    return np-next;
}
//if no queue found return null else it gets process from reaady queue with using dequeue 
process *nextProcess(void){
    if(queR.size==0){
        return NULL;  
    } 
    process *p = queR.front->data;
    dequeueProcess(&queR);
    return p;    
      
}
// for new processws to store it usses enqueue to store in temporary queue
void tempque(void){
    while(processes[next].arrivalTime <= tt && next < np){
        temp[tempq++]=&processes[next++];
    }
}
/* if waiting processes are finished thn it moves I/O bursts to ready 
    stores I/O bursts into intermefiate array to sort by priority
*/
void waitingque(void){
    int w = queW.size;
    for(i=0;i<w;i++){
        process *front = queW.front->data;
        dequeueProcess(&queW);
        assert(front->bursts[front->currentBurst].length>=front->bursts[front->currentBurst].step);

        if(front->bursts[front->currentBurst].step == front->bursts[front->currentBurst].length){
            front->currentBurst++;
            temp[tempq++]=front;

        }
        else
            {enqueueProcess(&queW,front);} // if processes arrent finish it moves back to waiting quew
    }
}

// ready processes are moved to cpus with first come bases
void readyque(void){
    qsort(temp,tempq,sizeof(process*),compare);// sorts process in temp array by priority (fcfs)
    for(i=0;i<tempq;i++){
        enqueueProcess(&queR,temp[i]);
    }
    tempq = 0;
    for(i=0;i<NUMBER_OF_PROCESSORS;i++){
        if(cpu[i]==NULL){
            cpu[i] = nextProcess();
        }
    }
    
}
// move rrning processes to waiting if process are finished , it terminates their cpu burst
void finishedque(void){
    for(i=0;i<NUMBER_OF_PROCESSORS;i++){
        if(cpu[i]!= NULL){
            if(cpu[i]->bursts[cpu[i]->currentBurst].step ==cpu[i]->bursts[cpu[i]->currentBurst].length ){
                cpu[i]->currentBurst++;
                if(cpu[i]->currentBurst < cpu[i]->numberOfBursts){
                    enqueueProcess(&queW,cpu[i]);
                }
                else{
                    cpu[i]->endTime=tt;
                }
                cpu[i]= NULL;
            }
        }
    }
}

/* status
    increment current I/O bursts progress of 
*/
void statusW(void){
    j = queW.size;
    for(i=0;i<j;i++){
        process *front = queW.front->data;
        dequeueProcess(&queW);
        front->bursts[front->currentBurst].step++;
        enqueueProcess(&queW,front);
    }
}
void statusR(void){
      k=queR.size;
    for(i=0;i<k;i++){
        process *front = queR.front->data;
        dequeueProcess(&queR);
        front->waitingTime++;
        enqueueProcess(&queR,front);
    }
 }
  void statusrunning(){  
    for(i=0;i<NUMBER_OF_PROCESSORS;i++){
        if(cpu[i]!=NULL){
            cpu[i]->bursts[cpu[i]->currentBurst].step++;
        }
    }
  }




int main(void){
    int totaltt=0;
    int i;
    int readf=0;
    np = 0;
    tt = 0;
    bt = 0;
    wt = 0;
    cswitch=0;
    next =0;
    tempq = 0;
    
    initializeProcessQueue(&queR);
    initializeProcessQueue(&queW);
    for(i=0;i<NUMBER_OF_PROCESSORS;i++){
        cpu[i] = NULL;
    }
    while (readf == readProcess(&processes[np])){
        if(readf==1)
            np++;
        if(np>MAX_PROCESSES)
            break;
    }

    if(np==0){
        fprintf(stderr,"Not found ");
        return -1;
    }
    else if (np>MAX_PROCESSES){
        fprintf(stderr,"LIMIT reached of processes");
        return -1;
    }

    qsort(processes,np,sizeof(process),compareByArrival);

    while(1){
        tempque();
        finishedque();
        waitingque();
        readyque();
        statusW();
        statusR();
        statusrunning();
        bt += current();
        if(current()==0 && incoming()==0 && queW.size == 0)
            break;
        tt++;
    }

    for(i=0;i<np;i++){
        totaltt += processes[i].endTime - processes[i].arrivalTime;
        wt += processes[i].waitingTime;
    }
    double n = wt/(double) np;
    double m = totaltt/(double) np;
    double p = (bt * 100.0)/tt;
    printf( "Average waiting time: %f \n Average turnaround time : %f \n Average CPU utilization time: %f \n Time all processes finished: %d \n Context Switch: %d \n",
                n,m,tt,p,cswitch
    
    );

    printf("last process:");
    for(i=0;i<np;i++){
        if(processes[i].endTime == tt){
            printf("%d\n",processes[i].pid);
        }
        
    }




    return 0;





}







