#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <math.h>
// #define quanta 1000
// #define quanta 2000

pid_t id1, id2;
int status1, status2;
int isP1started, isP2started, isP1finished, isP2finished;

int main(int argc, char *argv[])  {
    long long n1, m1, n2, m2;
    n1 = atoi(argv[1]);
    m1 = atoi(argv[2]);
    n2 = atoi(argv[2]);
    m2 = atoi(argv[3]);

    // create all the shms and reset all flags manually then detach

    int matrix1id, matrix1_row_flagid, matrix2id, matrix2_column_flagid, outputid, dimensionsid;

    outputid = shmget((key_t)1000, 1024, 0666|IPC_CREAT); // 1024 chars only
    dimensionsid = shmget((key_t)1001, 3*sizeof(long long), 0666|IPC_CREAT); // 1024 chars only


    matrix1id = shmget((key_t)2041, n1*m1*sizeof(long long), 0666|IPC_CREAT);
    matrix1_row_flagid = shmget((key_t)2042, n1*sizeof(int), 0666|IPC_CREAT);

    matrix2id = shmget((key_t)3041, n2*m2*sizeof(long long), 0666|IPC_CREAT);
    matrix2_column_flagid = shmget((key_t)3042, m2*sizeof(int), 0666|IPC_CREAT);

    int *matrix1_flag = shmat(matrix1_row_flagid, NULL, 0);
    int *matrix2_flag = shmat(matrix2_column_flagid, NULL, 0);

    for(int j=0;j<n1;j++)
        matrix1_flag[j]=0;

    for(int j=0;j<m2;j++)  
        matrix2_flag[j]=0;

    shmdt(matrix1_flag);
    shmdt(matrix2_flag);




        id1 = fork();
        if(id1==0)  {

            char* ch[] = {"./P1Scheduled.out", argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], NULL};
            printf("%d",execv("./P1Scheduled.out", ch));
        }
        waitpid(id1, &status1, WUNTRACED);

        id2 = fork();

        if(id2==0)  {
            char* ch[] = {"./P2Scheduled.out", NULL};
            printf("%d",execv("./P2Scheduled.out", ch));

        }
        waitpid(id2, &status2, WUNTRACED);
        isP1started = 0;
        isP1finished = 0;
        isP2started = 0;
        isP2finished = 0;
        struct timespec p1start, p1stop, p2start, p2stop, start, stop, temp1, temp2;
        long exec1 = 0L; //exec time in micros
        long exec2 = 0L;
        clock_gettime(CLOCK_REALTIME, &start);
        do  {
            if(status1!=0)  {
                if(!isP1started)  { // if executing P1 for the first time 
                    isP1started = 1;
                    clock_gettime(CLOCK_REALTIME, &p1start);
                }
            kill(id1, SIGUSR1);
            kill(id1, SIGCONT);
            clock_gettime(CLOCK_REALTIME, &temp1);
            waitpid(id1, &status1 ,WUNTRACED);
            clock_gettime(CLOCK_REALTIME, &temp2);
            exec1 +=  (temp2.tv_sec - temp1.tv_sec)*1000000000 + temp2.tv_nsec-temp1.tv_nsec;
            }

            if(status1 == 0 && !isP1finished)  {  // P1 just finished
                    isP1finished = 1;
                    clock_gettime(CLOCK_REALTIME, &p1stop);
            }

            if(status2!=0)  {
                 if(!isP2started)  { // if executing P2 for the first time  
                    isP2started = 1;
                    clock_gettime(CLOCK_REALTIME, &p2start);
                }
            kill(id2, SIGUSR2);
            kill(id2, SIGCONT);
            clock_gettime(CLOCK_REALTIME, &temp1);
            waitpid(id2, &status2 ,WUNTRACED);
            clock_gettime(CLOCK_REALTIME, &temp2);
            exec2 += (temp2.tv_sec - temp1.tv_sec)*1000000000 + temp2.tv_nsec-temp1.tv_nsec;
            }

            if(status2 == 0 && !isP2finished)  {  // P2 just finished
                    isP2finished = 1;
                    clock_gettime(CLOCK_REALTIME, &p2stop);
            }


        } while(status1 != 0 || status2 != 0);
        clock_gettime(CLOCK_REALTIME, &stop);
        long p1turnaround = (p1stop.tv_sec - p1start.tv_sec)*1000000000 + p1stop.tv_nsec-p1start.tv_nsec;
        long p2turnaround = (p2stop.tv_sec - p2start.tv_sec)*1000000000 + p2stop.tv_nsec-p2start.tv_nsec;
        long totalturnaround = (stop.tv_sec - start.tv_sec)*1000000000 + stop.tv_nsec-start.tv_nsec;
       
        printf("Total Turnaround Time %ld nanoseconds\n", totalturnaround);
        printf("Context Switching Time %ld nanoseconds\n", totalturnaround-exec1-exec2);

        printf("P1 Turnaround Time %ld nanoseconds\n", p1turnaround);
        printf("P1 Execution Time %ld nanoseconds\n", exec1);
        printf("P1 Waiting Time %ld nanoseconds\n", p1turnaround-exec1);

        printf("P2 Turnaround Time %ld nanoseconds\n", p2turnaround);
        printf("P2 Execution Time %ld nanoseconds\n", exec2);
        printf("P2 Waiting Time %ld nanoseconds\n", p2turnaround-exec2);
                
        shmctl(matrix1id, IPC_RMID, NULL);
        shmctl(matrix2id, IPC_RMID, NULL);
        shmctl(matrix1_row_flagid, IPC_RMID, NULL);
        shmctl(matrix2_column_flagid, IPC_RMID, NULL);
        shmctl(outputid, IPC_RMID, NULL);
        shmctl(dimensionsid, IPC_RMID, NULL);   
 
    return 0;
}