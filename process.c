/*
Family Name: Gandhi
Given Name(s): Achal    
Student Number: 218319111
EECS Login ID (the one you use to access the red server): achal167
YorkU email address (the one that appears in eClass): achal167@my.yorku.ca
*/

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define SIZE 101 //defining size for array we create to store process

int n; // this variable is to traverse through database 
int pid; //pipe tracking initiated
int i ;
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



int main(int argc, char* argv[]){
   
    
    // if there are no dataset available, loop process wont start
    if(argc < 2){
        fprintf(stderr,"usage:  <filename1><filename2> ...\n");
        exit(-1);
    }
    
    

    else{
        n=1;
        int fd[argc-1][2]; // 2D pipe to track read write 

        arr = (Array*) malloc(sizeof(Array)*argc); // allocating required memory to store result in array

        while(n<argc){
            char *file = argv[n]; // getting file from command line
            fc = fopen(file,"r"); // opening file to read
            
            //if no file found 
            if(fc == NULL){
                fprintf(stderr,"no file detected\n");
                exit(-1);
            }
            

            //if pipe is not initiated 
            if(pipe(fd[n-1])<0){
                fprintf(stderr,"pipe is not initiate\n");
            }


            // fork process
            // if fork process fails
            if ((pid=fork())<0){
                fprintf(stderr,"Fork failed");
               
            }

            // starting child process
            if (pid==0){
                close(fd[n-1][0]);
                double x;
                double min = INFINITY;
				double max = -INFINITY;

                double sum = 0.0;
                double diff = 0.0;
                char filename1[SIZE];               



                //until end of file 
                while((fscanf(fc,"%s",filename1))!=EOF){
                    
                   x = atof(filename1);// read file 
					 
					 if (x < min)
					 	 min = x; 
					 
					 if (x > max)
					 	 max = x; 


 
					
                    }
                sum = max + min;
                diff = max - min;
               
                
                
                strcpy(arr[n-1].filename,argv[n]); //copying filename to array for result

                // storing min,max,sum,diff value to array for parent process
                arr[n-1].min = min; 
                arr[n-1].max = max;
                arr[n-1].sum = sum;
                arr[n-1].diff = diff;

                //after storing result , child to write to the parent
                write(fd[n - 1][1],&arr[n - 1],sizeof(Array));

                close(fd[n - 1][1]);//closing writing 
                fclose(fc); // closing file
                exit(0); 



            }else 
            {
                // parent process starts from here
                
                
                close(fd[n-1][1]);

                // waiting all files to read, at the end of last file parents read from child 
                if(n+1==argc){
                      double fmin = INFINITY;
					  double fmax = -INFINITY;
                      

                        // for loop from i=0 to last file, copy the information to parent after all child process are finished
                      for(i=0;i<argc-1;i++){
                        wait(NULL);
                        read(fd[i][0],&arr[i],sizeof(Array));
                        close(fd[i][0]);
						printf("%s   SUM = %f  DIF = %f  MIN = %f  MAX = %f\n", arr[i].filename, arr[i].sum, arr[i].diff, arr[i].min, arr[i].max);

                        // getting minimum and maximum from results
                        if(arr[i].min<fmin)
                            fmin = arr[i].min;
                        

                        if(arr[i].max>fmax)
                            fmax = arr[i].max;
                        

                        if(i==argc-2)
                            printf("MINIMUM = %f\tMAXIMUM = %f\n",fmin,fmax);

                      }    
                   
                }
            }

            n++;//file count ++
        }
       
    }
   
}