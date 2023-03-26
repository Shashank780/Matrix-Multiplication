Group 52
SATVIK OMAR		        f20190933@hyderabad.bits-pilani.ac.in
RAJ TRIPATHI		    f20190869@hyderabad.bits-pilani.ac.in
PRANAVI GUNDA		    f20191068@hyderabad.bits-pilani.ac.in
PRANJAL JASANI		    f20190831@hyderabad.bits-pilani.ac.in
SHASHANK PRATAP SINGH	f20190956@hyderabad.bits-pilani.ac.in
SURAJ RETHEESH NAIR	    f20200051@hyderabad.bits-pilani.ac.in

// Write all the terminal commands from the root directory


//Our code consists of 5 files P1.c P2.c P1Scheduled.c P2Scheduled.c group52_assignment2.c
//In order to compile and run P1 and P2 independently, we are using P1.c and P2.c which is just slight modifications of P1Scheduled.c and P2Scheduled.c to make independent compilation and run of P1 and P2 possible

// 3 otherfiles, wrapperP1.c, wrapperP2.c and wrapperScheduler.c were used to generate the csv files required for the plots
// the report pdf and the csv files can be found in the Documentation folder


The Utilities folder contains all the .ipynb files used to generate the plots. These may require some modifitions before running on a new system.

// To generate input files by the name of matrix1.txt and matrix2.txt, use the following code(depending on your system)
python Utilities/MatrixGen.py i j k
OR 
python3 Utilities/MatrixGen.py i j k
a file named matrixes.txt will also be generated which contains an ixk matrix which is the product of ixj matrix1 and jxk matrix2

// To check if the output by the programs is correct, compare with with matrixres.txt using the utility compareFiles.c

// To run P1 and P2 separately please use the P1.c and P2.c files to compile and run. 

//To run the P1 separately, please use the following commands:
gcc -g -pthread P1.c -o P1.out -lm
./P1.out i j k matrix1.txt matrix2.txt output.txt

NOTE: To use wrapperP1, modify P1 to take an additional command line argument, ie thread count

//To run the P2 separately, please use the following commands:
gcc -g -pthread P2.c -o P2.out -lm
./P2.out


//To run the Scheduler compile P1Scheduled.c and P2Scheduled.c and run as given below:
gcc -g -pthread P1Scheduled.c -o P1Scheduled.out -lm
gcc -g -pthread P2Scheduled.c -o P2Scheduled.out -lm
gcc -g -pthread group52_assignment2.c -o group52_assignment2.out -lm
./group52_assignment2.out i j k matrix1.txt matrix2.txt output.txt

//To change time quanta, change the macro 'quanta', defined in P1Scheduled and P2Scheduled and recompile
