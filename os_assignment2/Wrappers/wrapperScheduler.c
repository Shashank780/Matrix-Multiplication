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
    FILE *fp1 = fopen("Documentation/performance3_P1_1ms.csv", "w");
    FILE *fp2 = fopen("Documentation/performance3_P2_1ms.csv", "w");
    fprintf(fp1, "P1 Turnaround Time, Total Turnaround Time, P1 Execution Time, P1 Waiting Time, Context Switch Overhead, Workload\n");
    fprintf(fp2, "P2 Turnaround Time, Total Turnaround Time, P2 Execution Time, P2 Waiting Time, Context Switch Overhead, Workload\n");
    fflush(fp1);
    fflush(fp2);
    long long n1, m1, n2, m2;
    n1 = atoi(argv[1]);
    m1 = atoi(argv[2]);
    n2 = atoi(argv[2]);
    m2 = atoi(argv[3]);

    for(int i = 0; i<50; i++)  {
        // n1 *= 2;
        // m1 *= 2;
        // n2 *= 2;
        // m2 *= 2;
        n1 += 20 + rand()%20;
        m1 += 20 + rand()%20;
        n2 = m1;
        m2 += 30;
        char ch1[6], ch2[6], ch3[6];
        sprintf(ch1, "%lld", n1);
        sprintf(ch2, "%lld", m1);
        sprintf(ch3, "%lld", m2);
        pid_t pthid = fork();
        if(pthid==0)  {
            printf("%d", execlp("python3", "python3", "Utilities/MatrixGen.py", ch1, ch2, ch3, NULL));
        }
        waitpid(pthid, NULL, 0);


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

            char* ch[] = {"./P1Scheduled.out", ch1, ch2, ch3, argv[4], argv[5], argv[6], NULL};
            //strcpy(argv[0],"./P1.out");
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
        fprintf(fp1, "%ld, %ld ,%ld ,%ld, %ld,  %lld %lld %lld\n", p1turnaround, (stop.tv_sec - start.tv_sec)*1000000000 + stop.tv_nsec-start.tv_nsec, exec1, p1turnaround-exec1, (stop.tv_sec - start.tv_sec)*1000000000 + stop.tv_nsec-start.tv_nsec - exec1 -exec2, n1, m1, m2);
        fprintf(fp2, "%ld, %ld ,%ld ,%ld, %ld,  %lld %lld %lld\n", p2turnaround, (stop.tv_sec - start.tv_sec)*1000000000 + stop.tv_nsec-start.tv_nsec, exec2, p2turnaround-exec2, (stop.tv_sec - start.tv_sec)*1000000000 + stop.tv_nsec-start.tv_nsec - exec1 -exec2, n1, m1, m2);
        fflush(fp1);
        fflush(fp2);
        shmctl(matrix1id, IPC_RMID, NULL);
        shmctl(matrix2id, IPC_RMID, NULL);
        shmctl(matrix1_row_flagid, IPC_RMID, NULL);
        shmctl(matrix2_column_flagid, IPC_RMID, NULL);
        shmctl(outputid, IPC_RMID, NULL);
        shmctl(dimensionsid, IPC_RMID, NULL);
    }
    
    
    
    fclose(fp1);
    fclose(fp2);
 
    return 0;
}