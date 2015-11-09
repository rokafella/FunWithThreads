Name: Rohit Kapoor
Email: kapoor.83@osu.edu

CSE 5441 Lab 1b - Pthread "persistent threads" model

How to execute?

-- The folder contains a Makefile. Go to the directory and type "make" and program will start compiling.
-- After the make command is done, execute "/a.out" and the program will start executing.
—- When prompted please enter the number of threads you want the program to execute with.

Files:

1) main.cpp is the main program file.
2) Makefile is for compiling the program with appropriate flags.
3) testgrid_1 is the data file which was provided to us.
4) Stats.txt include the running time of the program for different number of threads.

Output:

The program will output total number of boxes read, total columns and total rows in the file.
The next two lines will be the Minimum and Maximum values of the boxes in the last iteration.
Fourth line will be the total time taken by the convergence loop in seconds (calculated using chrono).
The last line will be the total number of iterations it took to converge.