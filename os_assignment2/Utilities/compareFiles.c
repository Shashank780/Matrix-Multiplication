#include <stdio.h>
#include <stdlib.h>

int main()  {
    FILE *fp1 = fopen("matrixres.txt", "r");
    FILE *fp2 = fopen("output.txt", "r");
    long num1, num2, c1, c2;
    long counter = 0;
    do  {
        c1 = fscanf(fp1, "%ld", &num1);
        c2 = fscanf(fp2, "%ld", &num2);
        if(num1 != num1) {
            printf("%ld %ld %ld", num1, num2, counter);
            exit(0);
        }
        counter ++;
    }
    while(c1 != EOF || c2 != EOF);

    printf("The result is correct!");
    fclose(fp1);
    fclose(fp2);
    return 0;
}