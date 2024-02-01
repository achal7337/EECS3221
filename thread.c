/*
Family Name: Gandhi
Given Name(s): Achal    
Student Number: 218319111
EECS Login ID (the one you use to access the red server): achal167
YorkU email address (the one that appears in eClass): achal167@my.yorku.ca
*/
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define SIZE 101 //defining size for array we create to store process

static int n; // thread count
FILE *fc; //file

// structure for array to store result from child processes.
    typedef struct Array{
        double min;
        double max;
        double sum;
        double diff;
        
        char filename[SIZE];
    } Array;

    Array *arr;

void *calculation (void cal){

    fc = fopen(cal,"r");
    if(fc==NULL){
         fprintf(stderr,"usage:  <filename1><filename2> ...\n");
         exit(-1);
    }
    char file[SIZE];
    double min = 0.;
    double max = 0.;
    double sum = 0.;
    double diff = 0.;
    double x;

    while ((fscanf(fc,"%s",file))!=EOF){
        
        x = atof(file);
        if (x < min)
                min = x;

        if (x > max)
                max = x;
    }
    sum = max + min;
    diff = max - min;
    strcpy(arr[n].filename, cal);


    arr[n].min = min;
    arr[n].max = max;
    arr[n].sum = sum;
    arr[n].diff = diff;



    n++;
    fclose(fc);
    pthread_exit(0);
}
    



int main(int argc, char* argv[]){
   
    
    // if there are no dataset available, loop process wont start
    if(argc < 2){
        fprintf(stderr,"usage:  <filename1><filename2> ...\n");
        exit(-1);
    }
    
    

    else{
        pthread_attr_t attr;
        int fd[argc-1]; 

        arr = (Array*) malloc(sizeof(Array)*argc); // allocating required memory to store result in array

        while(n<argc){
            char *file = argv[n]; // getting file from command line
            pthread_attr_init(&attr);
            pthread_create(&tid[n-1],NULL,calculation,cal)

            int j; 
            for(j=0;j<argc-1;j++){
                pthread_join(tid[j],NULL);
               
            }
            double fmin = 0.0;
            double fmax = 0.0;
            if (n+1==argc){
            for(i=0;i<argc-1;i++){
						printf("%s   SUM = %f  DIF = %f  MIN = %f  MAX = %f\n", arr[i].filename, arr[i].sum, arr[i].diff, arr[i].min, arr[i].max);

                        // getting minimum and maximum from results
                        if(arr[i].min<fmin)
                            fmin = arr[i].min;
                        

                        if(arr[i].max>fmax)
                            fmax = arr[i].max;
                        

                        if(i==argc-2)
                            printf("MINIMUM = %f\tMAXIMUM = %f\n",fmin,fmax);

                      }    
                   
                } n++;
            }
    } return 0;

}
     
    
   
