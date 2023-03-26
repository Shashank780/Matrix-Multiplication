#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <math.h>
int status1=0;
struct timespec start, stop;

int main(int argc, char *argv[])  
{
    char *filename = "performance.csv";
    FILE *fp = fopen(filename, "w");
    char temp1[6], temp2[6], temp3[6];
    long long n1, n2, n3;
    n1 = 10;
    n2 = 5;
    n3 = 10;
    fprintf(fp, "Number of Threads, Execution Time, Workload\n");
    for(int i =0; i<3; i++)  {
        sprintf(temp1, "%ld", (long)(n1*pow(10,i)));
        sprintf(temp2, "%ld", (long)(n2*pow(10,i)));
        sprintf(temp3, "%ld", (long)(n3*pow(10,i)));
        pid_t id = fork();
        if(id==0)  {
            printf("%d",execlp("python3","python3","MatrixGen.py", temp1, temp2, temp3, (char*)NULL));

        }
        waitpid(id,NULL,0);
       
        int N=1;//thread cap for now =50
        pid_t *pid;
        int status[1000];
        
        long exec1 = 0L; //exec time in micros
        pid= malloc(1000*sizeof(int));
        do{
            pid[N]=fork();
            char temp[6];
            sprintf(temp, "%d", N);
            char *ch[] = {"./P1.out", temp1, temp2, temp3, argv[1], argv[2], argv[3], temp, NULL};
            clock_gettime(CLOCK_REALTIME, &start);
            if(pid[N]==0)  {
                execv("./P1.out", ch);
            }
            wait(&status[N]);
            clock_gettime(CLOCK_REALTIME, &stop);
            fprintf(fp,"%d, %ld, %ld %ld %ld\n",N, (stop.tv_sec - start.tv_sec)*1000000000+ stop.tv_nsec-start.tv_nsec, (long)(n1*pow(10,i)),(long)(n2*pow(10,i)),(long)(n3*pow(10,i)));

            //printf("Total execution time for %d threads %ld ns \n", N,(stop.tv_sec - start.tv_sec)*1000000000+ stop.tv_nsec-start.tv_nsec);
            N++;

        }while(N<200);
    }
        
    
}
