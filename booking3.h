/* booking3.h
* Header file to be used with
* shmp3.cpp and shmc3.cpp
*/
#include <semaphore.h> //includes semaphore functions for parent and child processes to use
// header file needed for POSIX semaphore
struct TOUR {
	//components of a tour
	//most important are the seats_left
	char bus_number[6];
	char date[9];
	char title[50];
	int seats_left;
};
struct BUS {
	sem_t sem1; //semaphore variable that will hold our unnamed semaphore
	// semaphore to control access to tour data below
	TOUR tour1 = { "4321", "11262021", "Grand Canyon Tour", 20 }; //TOUR variable instantiation
} mybus; //reference to our BUS struct