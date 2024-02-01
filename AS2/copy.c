/*
Family Name:Gandhi   
Given Name:Achal    
Student Number:218319111
EECS Login ID (the one you use to access the red server): achal167
YorkU email address (the one that appears in eClass): achal167@my.yorku.ca
*/



#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#include <semaphore.h>
#define TEN_MILLIS_IN_NANOS 10000000

//bufSize
int bufSize = 0; 
// input file, output file , log file
FILE *input, *output, *log_file;
int i,j=0;
//number of in and out thread
int inthread, outthread = 0;
int q;
//input and output thread for main 
int *inputthread, *outputthread;

//buffer item
typedef struct {
    char data;
    off_t offset;
    char string[7];
} awbufferItem;

awbufferItem *buffer;

/*mutexes for file
 * lock for thread
 * ending of thread(finish)
 * increment in in thread
 * increament in out thread
 * counting threads, check if threads are decreasing 
 */
pthread_mutex_t start_lock;
pthread_mutex_t finish_lock;
pthread_mutex_t input_lock;
pthread_mutex_t output_lock;
pthread_mutex_t count_lock;


void thread_sleep() {
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = rand()%(TEN_MILLIS_IN_NANOS+1);
    nanosleep(&t,NULL);
}

// return index of full cell
int full_item(){
	for(i=0;i< bufSize;i++){
		if (strcmp(buffer[i].string, "filled") == 0){
			return i;
		}
	}
	return -1;
}

// index of empty
int empty_item(){
    for(i=0;i<bufSize;i++){
        if(strcmp(buffer[i].string,"empty")==0){
            return i;
        }
    }
    return -1;
}

 // takes input file in input thread, reads file then it copies from file to buffer
 
void *input_thread_func(void *param){
    thread_sleep();
    // variables
    int index,address,offcet;
    char byte;
    offcet = inthread++;// increase in thread count
    pthread_mutex_lock(&input_lock);
    pthread_mutex_unlock(&input_lock);
    
    do {
        pthread_mutex_lock(&start_lock); //mutex lock to acquires ownership of start_lock
        index = empty_item();

        while (index != -1){
            /*
             * reads from file then write it to log file of activity 
             * it produces in any buffer (empty) 
             */
            address = ftell(input);
			byte = fgetc(input);
            //  this writes to log file
			if (fprintf(log_file, "read_byte PT%d O%d B%d I-1\n", offcet, address, byte) < 0) {
				fprintf(stderr, "\tProvide Valid Log File!\n\tExiting Now!\n");
				exit(1);
			}
			
			if (byte == EOF){
				break;
			}
            // produce from file
			else{ 
				buffer[index].data = byte;
				buffer[index].offset = address;
				strcpy(buffer[index].string, "filled");
			}
			
			if (fprintf(log_file, "produce PT%d O%d B%d I%d\n", offcet, address, byte, index) < 0) {
				fprintf(stderr, "\tinvalid file \n\t exits \n");
				exit(1);
			}
			index = empty_item();
			
			thread_sleep();
        }
        pthread_mutex_unlock(&start_lock); // rellinquishes ownership pf start_lock
    } while(!feof(input));

    pthread_mutex_lock(&count_lock); // aquires ownership of count_lock
    q--; //decrement thread count
    pthread_mutex_unlock(&count_lock);// rellinquishes ownership of count_lock
    
    pthread_mutex_lock(&finish_lock); //acquires ownership of finish_lock
    inputthread[offcet] = 1; //when thread has finished it sets to 1
    pthread_mutex_unlock(&finish_lock); // rellinquishes ownership of finish_lock

    thread_sleep();
    pthread_exit(0);


}
// from buffer it copies to file
void *output_thread_func(void *param){

    thread_sleep();

    int p,offcet,address;
    int index = 0;
    char byte;

    pthread_mutex_lock(&output_lock);
    offcet = outthread++;
    pthread_mutex_unlock(&output_lock);
    
    do {
        index = full_item();
       if (index != -1){
			
			pthread_mutex_lock(&start_lock); //aquires ownership of mutex lock
			index = full_item(); 
			
			address = buffer[index].offset;
			byte = buffer[index].data;
		
			if (fprintf(log_file, "consume CT%d O%d B%d I%d\n", offcet, address, byte, index) < 0) {
				fprintf(stderr, "\tinvalid file \n\t exits \n");
				exit(1);
			}
			
			strcpy(buffer[index].string, "empty");
			buffer[index].offset = 0; // sets buffer index to 0 if its empty
			
			pthread_mutex_unlock(&start_lock); //relinquish ownersship o fmutex lock
			pthread_mutex_lock(&start_lock);
			
            // copy to file
			if (fseek(output, address, SEEK_SET) < 0) {
				pthread_mutex_unlock(&start_lock);
    			fprintf(stderr, "\tinvalid file \n\t exits \n");
				exit(1);
			}
			if (fputc(byte, output) == EOF) {
				pthread_mutex_unlock(&start_lock);
    			fprintf(stderr, "\tinvalid file \n\t exits \n");
				exit(1);
			}
            // activity log to log file
			if (fprintf(log_file, "write_byte CT%d O%d B%d I-1\n", offcet, address, byte) < 0) {
				fprintf(stderr, "\tProvide Valid Log File!\n\tExiting Now!\n");
				exit(1);
			}
		
			pthread_mutex_unlock(&start_lock);
		}

        thread_sleep();
    
        p = q; // assigning threadcounts to indicate it finish so process can exit
    
    } while (p > 0|| index != -1);

    pthread_mutex_lock(&finish_lock);
    outputthread[offcet] = 1;
    pthread_mutex_unlock(&finish_lock);

    pthread_exit(0);
} 


int main(int argc, char *argv[]){

    if(argc != 7){
        fprintf(stderr,"\tProvide valid Arguments!\n\tExiting Now!\n");
        exit(1);
    }

    pthread_mutex_init(&count_lock,NULL);
    pthread_mutex_init(&input_lock,NULL);
    pthread_mutex_init(&start_lock,NULL);
    pthread_mutex_init(&output_lock,NULL);
    pthread_mutex_init(&finish_lock,NULL);

    int t = 0;
     // reading arguments and assigning   
    int nIN = atoi(argv[1]);
    int nOUT = atoi(argv[2]);
    char *f_in = argv[3];
    char *f_copy = argv[4];
    bufSize = atoi(argv[5]);
    char *log_f = argv[6];
    q = nIN;


    inputthread = malloc(nIN *sizeof(int));
    outputthread = malloc(nOUT *sizeof(int));

    if(inputthread == NULL || outputthread == NULL){
        fprintf(stderr,"\t cant access threads\n");
        exit(1);
    }

    if(nIN < 1 || nOUT < 1 || bufSize < 1){
        fprintf(stderr,"\t Invalid values\n");
        exit(1);
    }

    log_file = fopen(log_f,"w");
    if(log_file == NULL){
        fprintf(stderr,"\t invali file\n");
        exit(1);
    }

    input = fopen(f_in,"r");
    if(input == NULL){
        fprintf(stderr,"\t invalid input file\n");
        exit(1);
    }

    output = fopen(f_copy,"w");
    if(output == NULL){
        fprintf(stderr, "\t invalid output file\n");
        exit(1);
    }
    pthread_t in_th[nIN];
    pthread_t out_th[nOUT];

    printf("\t creating buffer\n\t");

    buffer = (awbufferItem*) malloc(bufSize * sizeof(awbufferItem));
    for(t = 0;t<bufSize;t++)
        strcpy(buffer[t].string,"empty");
    
    printf("creating in threads..\n\t");

    for(t=0;t<nIN;t++)
        pthread_create(&in_th[t],NULL, (void *)input_thread_func, input);
    
    printf("creating out threads\n\t");

    for(t=0;t<nOUT;t++)
        pthread_create(&out_th[t],NULL, (void *)output_thread_func,output);
    
    printf("waiting for inthreads \n\t");
    
    for(t=0;t<nIN;t++)
        while(inputthread[t]==0);
    
    printf("waiting for out threads.. \n\t");

    for(t=0;t<nOUT;t++)
        while(outputthread[t]==0);
    
    
    for(t=0;t<nIN;t++)
        pthread_join(in_th[t],NULL);
    

    for(t=0;t<nOUT;t++)
        pthread_join(out_th[t],NULL);
    
    printf("copied successfully.\n");

    //close files and free buffer 
    fclose(input);
    fclose(log_file);
    fclose(output);
    free(buffer);
    free(inputthread);
    free(outputthread);

    exit(0);
  
}