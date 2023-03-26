// Only runs as a scheduled subprocess

// The program reads from the given input files
// The reading is parallelised using pthreads.

// Group 52
// Group Member Details:
// 1. Raj Tripathi         	2019B4A70869H f20190869@hyderabad.bits-pilani.ac.in
// 2. Satvik Omar          	2019B4A70933H f20190933@hyderabad.bits-pilani.ac.in
// 3. Pranavi Gunda        	2019B4A71068H f20191068@hyderabad.bits-pilani.ac.in
// 4. Pranjal Jasani       	2019B4A70831H f20190831@hyderabad.bits-pilani.ac.in
// 5. Shashank Pratap Singh 2019B4A70956H f20190956@hyderabad.bits-pilani.ac.in
// 6. Suraj Retheesh Nair  	2020A7PS0051H f20200051@hyderabad.bits-pilani.ac.in

// All the information is stored into shared memory(shm) which is later made available to P2Scheduled
// Once all elements are written in shm, the program execution stops.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#define quanta 1000
// #define quanta 1000

int numberOfThreads;
pthread_mutex_t lock;
typedef struct{
    char *inp1;
    char *inp2;
    long long *matrix;
    long long *matrix2;//to store the shared memory elements of column in matrix2
    int *flag1;
    int *flag2;
    long long start;
    int count;
    int threadCount;
    int columns;
    int rowStart;

    //column code added below
    int count2; // number of columns per thread for matrix 2
    int rows; //number of rows for matrix 2
    int column2;//number of column for matrix 2
    int colStart;
    int countvar;


} params;

long long* index_matrix1;//preprocess matrix 1
long long* index_matrix2;//preprocess matrix 2

void *writeRow(void *p1)  
{

    params args = *((params*) p1);
    FILE *inp1 = fopen(args.inp1, "r");

    long long num;

    fseek(inp1,args.start,SEEK_SET);

    for(int i=0;i<args.count*args.columns;i++)
    {
        fscanf(inp1,"%lld",&num);
        args.matrix[args.threadCount*args.columns*args.rowStart +i]=num;
        if((i%args.columns)==args.columns-1)
            args.flag1[args.threadCount*args.rowStart+ (int)(i/args.columns)]=1;
   

    }

    fclose(inp1);
    
    FILE *inp2 = fopen(args.inp2, "r");


    for(int i = (args.threadCount*args.countvar); i<(args.threadCount*args.countvar + args.count2); i++)
    {
        int rowCount = 0;
        for(int j = 0; j<args.rows; j++)
        {
            fseek(inp2,(index_matrix2[j*args.column2 + i] - rowCount), SEEK_SET);
            fscanf(inp2,"%lld",&num);
            args.matrix2[j*args.column2 + i] = num;
            rowCount++;
        }
       args.flag2[i] = 1;
    }

    fclose(inp2);
}


void Index(long long x1, long long y1, long long x2, long long y2, char *inp1name, char *inp2name)   //preprocess both text files
{
      long long n1=x1;
      long long m1=y1;
      long long n2 = x2;
      long long m2 = y2;
      long long *index;
      index= malloc(sizeof(long long)*n1*m1);
      long long *index2;
      index2= malloc(sizeof(long long)*n2*m2);
      index_matrix2 = index2;
      
      FILE *inp1=fopen(inp1name,"r");
      int ch = fgetc(inp1);
      long long pos=0;
      
      int row=0,col=0;
      if(inp1==NULL)
      {
        printf("error\n");
      }
      
      while(ch!=EOF && row<n1 && col<m1)
      {
         
         if(ch==32)
         {
            
            index[row*m1 + col]=pos;
            col++;
         }
         if(ch==10)
         {
            
            index[row*m1 +col]=pos-1;
            row++;
            pos=-1;
            col=0;
          }
         pos++;
         ch=fgetc(inp1);
      }
      fclose(inp1);
            index_matrix1=index;

      FILE *inp2=fopen(inp2name,"r");
      row = 0;
      col = 0;
      ch = fgetc(inp2);
      long long pos2 = 0;
      index2[0] = -1;
      while(ch!=EOF && row<n2 && col<m2)
      {
         
         if(ch==32)
         {
            index2[row*m2 + col + 1]=pos2;
            col++;
         }
    
         if(ch==10)
         {
           
            pos2 = pos2 + 1;
            index2[row*m2 + col + 1]=pos2++;
            ch=fgetc(inp2);
             row++;
            col = 0;
           
          }
         ch=fgetc(inp2);
         pos2++;
      }
      fclose(inp2);
    
}

void sigHandler(int sig)  {

    if(sig==SIGUSR1)  {
        ualarm(quanta,0);
        

    }
    if(sig==SIGALRM)  {
        kill(getpid(), SIGSTOP);

        
    }


}

int main(int argc, char *argv[])  {

    signal(SIGUSR1,sigHandler);
    signal(SIGALRM, sigHandler);
    kill(getpid(), SIGALRM);

    if(argc !=7)  {
       printf("Invalid command line arguments");
       exit(-1);
    }
    long long n1, m1; // nxm matrix
    long long n2, m2;  // m1=n2
    n1 = atoi(argv[1]);
    m1 = atoi(argv[2]);
    n2 = atoi(argv[2]);
    m2 = atoi(argv[3]);
    int N=8;
    int *rowCalc;
    rowCalc=malloc(sizeof(int)*n1);
    FILE *inp1 = fopen(argv[4], "r");
    FILE *inp2 = fopen(argv[5], "r"); // later on make it 5

    int matrix1id, matrix1_row_flagid, matrix2id, matrix2_column_flagid, outputid, dimensionsid;

    outputid = shmget((key_t)1000, 1024, 0666); // 1024 chars only
    dimensionsid = shmget((key_t)1001, 3*sizeof(long long), 0666); // 1024 chars only


    matrix1id = shmget((key_t)2041, n1*m1*sizeof(long long), 0666);
    matrix1_row_flagid = shmget((key_t)2042, n1*sizeof(int), 0666);

    matrix2id = shmget((key_t)3041, n2*m2*sizeof(long long), 0666);
    matrix2_column_flagid = shmget((key_t)3042, m2*sizeof(int), 0666);

    long long *sharedMatrix1,*sharedMatrix2, *dimensions;
    int  *matrix1_row_flags, *matrix2_column_flags;
    char *output;

    sharedMatrix1 = shmat(matrix1id, NULL, 0);
    matrix1_row_flags = shmat(matrix1_row_flagid, NULL, 0);

    sharedMatrix2 = shmat(matrix2id, NULL, 0);
    matrix2_column_flags = shmat(matrix2_column_flagid, NULL, 0);

    output = shmat(outputid, NULL, 0);
    dimensions = shmat(dimensionsid, NULL, 0);
    dimensions[0] = n1;
    dimensions[1] = m1;
    dimensions[2] = m2;
    strcpy(output,argv[6]);

    Index(n1,m1,n2,m2, argv[4], argv[5]);

       
    fclose(inp1);
    fclose(inp2);

    rowCalc[0]=0;
    for(int i=1;i<n1;i++)
    {
        rowCalc[i]=(rowCalc[i-1] + index_matrix1[m1*(i-1)+m1-1]+2);
    }
    

    numberOfThreads=N;
    pthread_t *tid = malloc(sizeof(pthread_t)*N);
    params *p1=malloc(sizeof(params)*N);
    for(int i=0;i<N;i++)
    {

        p1[i].matrix=sharedMatrix1;
        p1[i].matrix2=sharedMatrix2;
        p1[i].flag1=matrix1_row_flags;
        p1[i].flag2=matrix2_column_flags;
        p1[i].start=rowCalc[i*((int)(n1/N))]; 
        p1[i].columns=m1;
        p1[i].threadCount = i;
        p1[i].rowStart=(int)n1/N; 
        p1[i].rows = n2;
        p1[i].column2 = m2;
        p1[i].colStart=(int)m2/N; 
        p1[i].countvar = ((int)(m2/N)); 
        p1[i].inp1 = argv[4];
        p1[i].inp2 = argv[5];
        
        if(i!=N-1) 
        {


            p1[i].count=((int)(n1/N));
            p1[i].count2 = ((int)(m2/N));
            if(N>n1)  {
                if(i<n1)  {
                    p1[i].count = 1;
                    p1[i].start=rowCalc[i];
                    p1[i].rowStart=1;
                }

            }
            if(N>m2)  {
                if(i<m2)  {
                    p1[i].count2 = 1;
                    p1[i].colStart=1; 
                    p1[i].countvar = 1;

                }

            }

            pthread_create(&tid[i],NULL,writeRow,&p1[i]);
        }
        else 
        {
            p1[i].count=((int)(n1/N) + n1%N);
            p1[i].count2 = ((int)(m2/N) + m2%N);
           if(N>n1)  {
                if(i<n1)  {
                    p1[i].count = 1;
                    p1[i].start=rowCalc[i];
                    p1[i].rowStart=1;
                }
                else
                    p1[i].count=0;

            }
            if(N>m2)  {
                if(i<m2)  {
                    p1[i].count2 = 1;
                    p1[i].colStart=1; 
                    p1[i].countvar = 1;
                }
                else
                    p1[i].count2=0;
            }
            pthread_create(&tid[i],NULL,writeRow,&p1[i]);
        }     
    }

    for(int i=0;i<N;i++)
    {
        pthread_join(tid[i],NULL);
    }

    return 0;


}
