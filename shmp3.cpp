/* shmp3.cpp */

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

using namespace std;

//defines number of children which is 3
#define NCHILD 3

//function prototypes outlined here
int shm_init( void * );
void wait_and_wrap_up( int [], void *, int );
void rpterror( char *, char * );

int main(int argc, char *argv[])
{
	int child[NCHILD], i, shmid; //holds PIDs of children, child number, and shared memory ID
	void *shm_ptr; //this is the shared memory pointer
	char ascshmid[10], pname[14]; //holds value of 'shmid' as character array, holds process name

	//sem_init will create out unnamed semaphore
	//the first argument is where our semaphore will be stored in memory
	//in our case we have created a semaphore variable in our BUS struct in booking3.h
	//we can access the memory address of this variable by &(mybus.sem1)
	//the second argument takes an integer of 0 or 1
	//1 indicates that the semaphore will be shared between processes
	//because we are using 1, we need to copy the memory into our shared memory segment
	//we do this below in shm_init()
	//the third argument is what the semaphore value will be initialized to which is 1
	//the if statement checks if sem_init was successful
	//if it was not it will print an error message
	if(sem_init(&(mybus.sem1), 1, 1) == -1) {
		perror("Semaphore failed to initialize."); //print error message
		exit(5); //exit code 5 indicates semaphore creation failed
	}

	shmid = shm_init(shm_ptr); //calls function shm_init which initializes our shared memory segment
	sprintf (ascshmid, "%d", shmid); //stores value of shmid into character array using sprintf

	//displays to console the components of BUS struct
	cout << "Bus " << mybus.tour1.bus_number << " for "
		<< mybus.tour1.title << " on " << mybus.tour1.date << ", "
		<< mybus.tour1.seats_left << " seats available. " << endl;
	cout << "Booking begins: " << endl << endl;

	//for loop to iterate until 3 (NCHILD), fork each time to create that many children
	for (i = 0; i < NCHILD; i++) {
		child[i] = fork(); //forking here and storing PID in child[i]
		switch (child[i]) { //switch statement instead of if statement
			case -1: //case -1 means that child[i] == -1 which means the fork failed
				sprintf (pname, "child%d", i+1); //storing child number as process name into pname
				//we do i+1 because the child number begins at 0
				rpterror ((char *)"fork failed", pname); //prints error message using the function rpterror below
				exit(1); //exits with code 1 to demonstrate error with forking and terminate program
			case 0: //case 0 means that child[i] == 0 which means the fork was a success
				sprintf (pname, "shmc%d", i+1); //storing child number as process name into pname
				//we do i+1 because the child number begins at 0
                //exec called here to execute shmc2.cpp passing in process name and shmid as a character array
				execl("shmc3", pname, ascshmid, (char *)0); 
				//if exec for some reason failed then rpterror will be called again to display that exec failed 
                //the reason rpterror does not execute if exec succeeds is because the child process will go into a new program shmc2.cpp
                //that is why we do not need to make an if statement saying if exec failed then we display an error
				rpterror ((char *)"execl failed", pname);
				//exit code 2 means exec failed and the program will terminate
				exit (2);
		}
	}
	//wait_and_wrap_up will wait for the child processes to terminate so that the parent can finish
	wait_and_wrap_up (child, shm_ptr, shmid);
}

//responsible for initializing our shared memory segment
int shm_init(void *shm_ptr)
{
	int shmid; //shared memory segment id container

	//shmget will initialize a shared memory segment and return an identifier for that shared memory segment
    //the first paramter is to generate a specific key for this shared memory segment
    //this is done using ftok which generates unique keys base on path and id
    //the second paramter will identift the size of our shared memory segment
    //in this case it is the size of our struct BUS
    //the third paramater will outline the read/write permissions
    //IPC_CREAT will identify whether this shared memory exists or not
    //if it does not then it will create it
    //shmget returns an integer which is stored in our shmid variable (shared memory ID)
	shmid = shmget(ftok(".",'u'), sizeof(BUS), 0600 | IPC_CREAT);
	//if shmget returned -1 then the system call has failed and we will display error
	if (shmid == -1) {
		perror ("shmget failed"); //prints error message to console
		exit(3); //exit code 3 delineates shmget failure and terminates program
	}
	//shmget simply creates our shared memory segment, but we cannot use it until we attach ourselves
    //we do this using the system call shmat
    //first paramter will take the shared memory segment ID which chooses our desired shared memory
    //second paramter is a void pointer 0, meaning the system will choose which address to attach it at
    //third paramter is 0 because it is based on the second parameter and our shmaddress is 0 as well
    //shmat returns a void pointer and is stored in our shm_ptr
	shm_ptr = shmat(shmid, (void * ) 0, 0);
	//if shmat fails it returns a void pointer -1
	if (shm_ptr == (void *) -1) {
		perror ("shmat failed"); //display error message to console that shmat failed
		exit(4); //exit code 4 delineates taht shmat failed
	}
	//memcpy will copy what is in its second paramter to its first parameter
    //that means we will take the data from our bus struct object and copy it into our shmptr
    //we must type cast it to a void pointer as that is the type of our shmptr
    //the third parameter will define the size of the the memory in second parameter
    //we do this so that all processes in the IPC system can use the data via our shared memory segment
	memcpy (shm_ptr, (void *) &mybus, sizeof(BUS) );
	//returns shared memory segment ID
	return (shmid);
}


void wait_and_wrap_up(int child[], void *shm_ptr, int shmid)
{
	//wait_rtn will hold the PID of the child returned by the wait system call
    //w is simply the child number
    //ch_active is the number of active children at the given time of the program's life cycle
	int wait_rtn, w, ch_active = NCHILD;
	while (ch_active > 0) { //while loop will wait for all active children which are greater than 0
		wait_rtn = wait( (int *)0 ); //wait system call here is waiting for child to terminate
		//it will store the child PID returned by wait in wait_rtn
		for (w = 0; w < NCHILD; w++) //for loop to iterate through children
			if (child[w] == wait_rtn) { //if child PID is equal to child PID returned by wait system call
				//then we will decrement the number of active children
				ch_active--;
				break; //break will take us out of the for loop and back into the while loop
			}
		}
	cout << "Parent removing shm" << endl;
	//shmdt detaches the program from the shared memory segment
    //parameter is our shared memory ptr to define which shared memory segment we are detaching from
	shmdt (shm_ptr);
	//shmctl destroys the shared memory segment
    //first parameter is the shared memory segment ID
    //second parameter is IPC_RMID which means it will destroy the shared memory segment
    //third parameter is a pointer to the shmid_ds struct
	shmctl (shmid, IPC_RMID, (struct shmid_ds *) 0);
	sem_destroy(&(mybus.sem1));
	exit (0); //exit code 0 means the program was a success
}

//rpterror is to display error messages
void rpterror(char *string, char *pname)
{
	char errline[50]; //character array to store errors

	sprintf (errline, "%s %s", string, pname); //storing the string and pname into errline
	perror(errline); //displaying on console the error
}