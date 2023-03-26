// Only runs as a scheduled subprocess

// The program performs the matrix multiplication
// The multiplication is parallelised using pthreads.

// Group 52
// Group Member Details:
// 1. Raj Tripathi         	2019B4A70869H f20190869@hyderabad.bits-pilani.ac.in
// 2. Satvik Omar          	2019B4A70933H f20190933@hyderabad.bits-pilani.ac.in
// 3. Pranavi Gunda        	2019B4A71068H f20191068@hyderabad.bits-pilani.ac.in
// 4. Pranjal Jasani       	2019B4A70831H f20190831@hyderabad.bits-pilani.ac.in
// 5. Shashank Pratap Singh 2019B4A70956H f20190956@hyderabad.bits-pilani.ac.in
// 6. Suraj Retheesh Nair  	2020A7PS0051H f20200051@hyderabad.bits-pilani.ac.in

// A finalResult[N1][N2] is maintained which stores the result.
// Once all elements are calculated, the result matrix is sent to be printed and the program execution stops.


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<sys/shm.h> 
#include <time.h>
#define quanta 1000
// #define quanta 1000

int N1,M1,N2,M2;



long long** finalResult;    //Stores the result matrix
int** resultMatrix_flag;    //Keeps track of what all elements in the result matrix is calculated
int* matrix1_row_flag;      //Keeps track of what all rows input of matrix 1 have been taken from P1
int* matrix2_column_flag;   //Keeps track of what all columns input of matrix 2 have been taken from P1

// Define a struct to be used by pthread_create() to call the function with parameters
typedef struct {
    int i;
    int n_threads;
    long long** Result;
    long long** A;
    long long** B;    
}arg_struct;

// To check if all elements have been calculated or not
int checkFlagMatrix(){
  for(int i=0;i<N1;i++){
    for (int j =0;j<M2;j++){
      if(resultMatrix_flag[i][j]==0){
        return 0;
      }
    }
  }
  return 1;
}


// Funtion to calculate matrix multiplication
void *multiply_threading(void* param){

    arg_struct *par;
    par = (arg_struct*)param;
    int thread_number = par->i;
    int NumberOfThreads = par->n_threads;
    // Calculate workload
    const int rows = N1; 
    const int n_operations = rows / NumberOfThreads; 
    const int remaining_operations = rows % NumberOfThreads;
    int numberofRowsInMatrix2 = N2;
    int NumberOfRowsInResult=N1; 
    int NumberOfColumnsInResult=M2;
    int start_row, end_row;

    if (thread_number == NumberOfThreads-1){
        //allotting the spare rows for the last thread  
        start_row = n_operations * thread_number;
        end_row = (n_operations * (thread_number + 1)) + remaining_operations;
    }
    else{
        start_row = n_operations * thread_number;
        end_row = (n_operations * (thread_number + 1)); 
    }
  
    for(int row=start_row; row < end_row; row++){
        for (int j = 0; j < NumberOfColumnsInResult; j++){
            par->Result[row][j] = 0;
            if(!resultMatrix_flag[row][j]){
                if(matrix1_row_flag[row] && matrix2_column_flag[j]){
                    for (int k = 0; k < numberofRowsInMatrix2; k++){
                        par->Result[row][j] += (par->A[row][k] * par->B[k][j]);
                        finalResult[row][j] = par->Result[row][j];
                        resultMatrix_flag[row][j]=1;
                    }
                }
            }
        }
    }
}

void sigHandler(int sig)  {

  if(sig==SIGUSR2)  {
    ualarm(quanta,0);
    

  }
  if(sig==SIGALRM)  {
      kill(getpid(), SIGSTOP);
    }


}

int main()
{   
    signal(SIGUSR2, sigHandler);
    signal(SIGALRM, sigHandler);
    kill(getpid(), SIGALRM);


    while(shmget((key_t)1001, 3*sizeof(long long), 0666)==-1)  {
    }

    int dimensionid = shmget((key_t)1001, 3*sizeof(long long), 0666); // 1024 chars only

    long long *dimensions = shmat(dimensionid, NULL, 0);
    N1 = dimensions[0];
    M1 = dimensions[1];
    N2 = dimensions[1];
    M2 = dimensions[2];

    finalResult=(long long**)malloc(N1*sizeof(long long*));
    for(int i=0;i<N1;i++)
      {finalResult[i]= (long long*)malloc(M2*sizeof(long long));}

    resultMatrix_flag=(int**)malloc(N1*sizeof(int*));
    for(int i=0;i<N1;i++)
      {resultMatrix_flag[i]= (int*)calloc(M2,sizeof(int));}
  
    matrix1_row_flag=(int*)malloc(N1*sizeof(int));

    matrix2_column_flag=(int*)malloc(M2*sizeof(int));

    
    long long **A2d=(long long**)malloc(N1*sizeof(long long*));
    for(int i=0;i<N1;i++)
      {A2d[i]= (long long*)malloc(M1*sizeof(long long));}  

    long long **B2d=(long long**)malloc(N2*sizeof(long long*));
    for(int i=0;i<N2;i++)
      {B2d[i]= (long long*)malloc(M2*sizeof(long long));}  

    //Max threads to be used
    int max_threads=8; 
    //If Max threads is greater than number of rows in result it will be brought to N1
    if(max_threads>N1){
      max_threads=N1;
    }
    while(shmget((key_t)2041, N1*M1*sizeof(long long), 0666)==-1)  {
    }
    int matrix1id = shmget((key_t)2041, N1*M1*sizeof(long long), 0666);
    long long *sharedMatrix1 = shmat(matrix1id, NULL, 0);
    
    while(shmget((key_t)3041, N2*M2*sizeof(long long), 0666)==-1)  {
    }
    int matrix2id = shmget((key_t)3041, N2*M2*sizeof(long long), 0666);
    long long *sharedMatrix2 = shmat(matrix2id, NULL, 0);
    
    while(shmget((key_t)1000, 1024, 0666)==-1)  {
    }
    int outputid = shmget((key_t)1000, 1024, 0666); // 1024 chars only
    char *output = shmat(outputid, NULL, 0);


    while(shmget((key_t)2042, N1*sizeof(int), 0666)==-1)  {
    }
    int matrix1_row_flagid = shmget((key_t)2042, N1*sizeof(int), 0666);
    
    while(shmget((key_t)3042, M2*sizeof(int), 0666)==-1)  {
    }

    int matrix2_column_flagid = shmget((key_t)3042, M2*sizeof(int), 0666);

    int *matrix1_flag = shmat(matrix1_row_flagid, NULL, 0);
    int *matrix2_flag = shmat(matrix2_column_flagid, NULL, 0);
       
        
        int count = 0;

        while(!checkFlagMatrix())
        {

          pthread_t *thread_array;
          int n_threads = max_threads;
          thread_array = malloc(n_threads * sizeof(pthread_t));
          arg_struct *args= malloc(sizeof(arg_struct)*n_threads);

          for (int i = 0; i < n_threads; i++) 
          { 
            long long **Result=(long long**)malloc(N1*sizeof(long long*));
            for(int i=0;i<N1;i++)  {
              Result[i]= (long long*)malloc(M2*sizeof(long long));
            }

            args[i].Result = Result;
            args[i].n_threads=n_threads;         
            args[i].i=i;  
            for(int j=0;j<N1;j++)  { 
                matrix1_row_flag[j] = matrix1_flag[j];
              }

            for(int j=0;j<M2;j++)
            {   
              matrix2_column_flag[j] = matrix2_flag[j];
            }

            for(int j=0;j<N1*M1;j++)
            {   
              A2d[j/M1][j%M1] = sharedMatrix1[j];
              
              if(j%M1==M1-1){
              }
            }

            for(int j=0;j<N2*M2;j++)
            {   
              B2d[j/M2][j%M2] = sharedMatrix2[j];
              
              if(j%M2==M2-1){
              }
            }
            args[i].A = A2d;
            args[i].B = B2d;
          // Initialize each thread with the function responsible of multiplying only a part of the matrices
           pthread_create(&thread_array[i], NULL, &multiply_threading,(void *)&args[i]);

        }

          for (int i = 0; i < n_threads; i++) 
          {
          // Wait until each thead has finished
            pthread_join(thread_array[i], NULL);
          }

      }

        FILE *fp;
        char *outname = malloc(1024);
        strcpy(outname,output);
        fp = fopen(outname, "w");
        for(int i =0;i<N1;i++){
            for(int j=0;j<M2;j++){
                fprintf(fp,"%lld ",finalResult[i][j]);
            }
            fprintf(fp,"\n");
        }
        fclose(fp);
      return 0;
}