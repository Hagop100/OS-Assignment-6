# OS-Assignment-6

Program Description

These programs interact with each other much like the previous assignment. In the last
assignment we used named semaphores to mitigate race condition. In this assignment however,
we will avoid issues presented by race condition using unnamed semaphores. Most of
everything works in a similar fashion with a few distinct changes. The parent program begins by
defining three children that it will fork(). Next, it initializes the shared memory segment via
shm_init() and proceeds to execute the three children processes. The children will then access
the BUS variable and will remove seats one by one. Our goal is to make sure the sem_wait()
and sem_post() are placed precisely around the critical section so that we avoid issues with
race condition.

We begin by analyzing the changes made in the booking header file. First we can see
there is a TOUR struct. This simply does what the BUS struct has done before and is there to
represent that we may have many more buses than just one. After all, the entire purpose of
using unnamed semaphores is because with named semaphores, if we did have many buses,
we would have to create an equal number of semaphores and name them. Unnamed
semaphores circumvent that issue. The most important aspect of the booking header file is the
semaphore variable in the BUS struct. This variable will be the memory address at which we
store our unnamed semaphore.

The parent process begins by doing what it normally does until we reach sem_init(). This
function will instantiate our unnamed semaphore. The first parameter will accept a pointer
semaphore variable. This is where we will store our unnamed semaphore in memory. Because
our BUS struct has such a variable delegated to handle such semaphore, we will use that exact
variable as storage. The second parameter signifies if our semaphore will be shared amongst
processes or threads. Ours will be shared amongst processes as we are using memcpy() down
below to copy our BUS variable to shared memory. That is why the second parameter will be 1
as opposed to 0. The third parameter signifies the initial value of our semaphore, which as we
know will be 1. This is because in the child process we decrement to 0 to access the critical
section and lock it, and then increment back to 1 to exit the critical section and unlock it. Lastly,
at the end of our parent program, we call sem_destroy() which also accepts a semaphore
pointer variable, and destroys our semaphore. This frees the memory used by the unnamed
semaphore.

The child process utilizes the unnamed semaphore to lock and unlock access to the
critical section. It does this via the same functions used in named semaphores: sem_wait() and
sem_post(). These two functions accept one parameter and that is our bus_ptr that points to the
semaphore memory space stored in the BUS struct’s semaphore variable. When sem_wait() is
called, it decrements the initial integer value of 1 to 0 and locks access to the critical section
where it sells a single seat. Once it is finished, it will increment this integer value of 0 back to 1
and unlock access to the critical section, allowing other processes to enter.
