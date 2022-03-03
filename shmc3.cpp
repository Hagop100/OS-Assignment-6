/* shmc3.cpp */

#include "booking3.h" //includes semaphore.h
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <memory.h>

//using standard namespace
using namespace std;

BUS *bus_ptr; //struct bus pointer
void *memptr; //used to store the void pointer returned by shmat
char *pname; //process name
int shmid, ret; //shared memory segment ID, return value of shmdt
//function prototypes
void rpterror(char *), srand(), perror(), sleep();
void sell_seats();

int main(int argc, char* argv[])
{
	if (argc < 2) { //argc should always be 3 
		fprintf (stderr, "Usage:, %s shmid\n", argv[0]); //prints the shmid
		exit(1); //exits due to error
	}
	//stores process name into pname
	pname = argv[0];
	sscanf (argv[1], "%d", &shmid); //stores value of argv[1] into address of shmid as an integer
    //we call shmat to attach our program to the shared memory segment
    //first paramter will take the shared memory segment ID which chooses our desired shared memory
    //second paramter is a void pointer 0, meaning the system will choose which address to attach it at
    //third paramter is 0 because it is based on the second parameter and our shmaddress is 0 as well
    //shmat returns a void pointer and is stored in our shm_ptr
	memptr = shmat (shmid, (void *)0, 0);
	if (memptr == (char *)-1 ) { //if memptr is -1 then shmat failed
		rpterror ((char *)"shmat failed"); //prints error message
		exit(2); //exit means shmat failed and program terminates
	}
	//memptr is a void pointer and must be type casted to a struct BUS pointer
	bus_ptr = (struct BUS *)memptr;
	sell_seats(); //sells seats (see below for detailed comments)
	//detaches program from the shared memory segment and stores value in ret
	ret = shmdt(memptr);
	exit(0); //program is a success and terminates
}

void sell_seats()
{
	int all_out = 0; //this means we are all out of seats
	srand ( (unsigned) getpid() ); //srand will select a random number using PID as seed
	while ( !all_out) {

		//sem_wait accepts our semaphore pointer
		//in this case we stored it our BUS struct
        //it ensures that only one process may enter the critical section of our code
        //in this case it is the selling of our seats
        //once a process enters this section, then other process are locked out
        //when the first process enters the critical section it will decrement the 
        //initial value of 1 to 0 which will disallow entry for the other processes
		sem_wait(&(bus_ptr->sem1));

		/* loop to sell all seats */
		if (bus_ptr->tour1.seats_left > 0) { //if we have more seats than 0
            //sleep will pause the program for given number of seconds
            //in this case with rand() it will be from 1 - 2 seconds randomly
			sleep ( (unsigned)rand()%2 + 1);
			//decrement number of seats as this is where we "sell" the seats
			bus_ptr->tour1.seats_left--;
			//pause program from 1 - 5 seconds randomly
			sleep ( (unsigned)rand()%5 + 1);
			//display number of seats remaining
			cout << pname << " SOLD SEAT -- "
			<< bus_ptr->tour1.seats_left << " left" << endl;
		}
		else { //if bus_ptr->seats == 0 then increment all_out
			all_out++; //this is to break us out of the while loop
			cout << pname << " sees no seats left" << endl; //print no seats left
		}

		//sem_post will accept the address of our semaphore variable
		//in this case we stored it our BUS struct
		//sem_post will unlock the door for other processes to enter the critical section above
        //when the process that entered the critical section is finished
        //it will increment the value from 0 back to 1 which will allow entry for the
        //other processes
		sem_post(&(bus_ptr->sem1));

		sleep ( (unsigned)rand()%5 + 1); //pause program from 1 - 5 seconds randomly
	}
}
void rpterror(char* string)
{
	char errline[50]; //character array to store errors
	sprintf (errline, "%s %s", string, pname); //storing the string and pname into errline
	perror (errline); //displaying on console the error
}