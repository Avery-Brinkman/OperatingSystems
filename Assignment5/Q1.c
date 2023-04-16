#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Number of customers to arrive
#define NUM_CUSTOMERS 10
// Number of seats in the waiting room
#define NUM_SEATS 5

// Mutex for getting number of free seats
pthread_mutex_t accessWRSeats = PTHREAD_MUTEX_INITIALIZER;

// Notifies that the barber is ready
sem_t barberReady;
// Notifies barber that a customer is waiting
sem_t custReady;
// Notifies the customer getting their haircut that the barber is done
sem_t doneCutting;

// Number of free seats for customers
int numFreeSeats = NUM_SEATS;
// Whether or not the barber is sleeping
int isSleeping = 0;

void* barber(void* arg) {
  while (1) {
    // Check if the waiting room is empty
    pthread_mutex_lock(&accessWRSeats);
    if (numFreeSeats == NUM_SEATS) {
      // If empty, go to sleep
      isSleeping = 1;
      pthread_mutex_unlock(&accessWRSeats);

      // Wait for customer
      printf("Barber sleeping... ZZZzzz...\n");
      sem_wait(&custReady);
    } else {
      // Update waiting room seat count.
      numFreeSeats += 1;
      // Set barber to be awake
      isSleeping = 0;
      pthread_mutex_unlock(&accessWRSeats);

      // Cut hair for a random amount of time
      printf("Cutting hair.\n");
      sleep(rand() % 5 + 1);

      // Notify the customer that the barber is done
      printf("Done cutting hair.\n");
      sem_post(&doneCutting);

      // Notify waiting customers that the barber is ready.
      sem_post(&barberReady);
    }
  }
}

void* customer(void* arg) {
  int id = (int)(int*)arg;
  // Come to barber after random amount of time
  sleep(rand() % 10 + 1);

  // Check for free seats
  pthread_mutex_lock(&accessWRSeats);
  if (numFreeSeats > 0) {
    // Take a seat and wait
    numFreeSeats -= 1;
    printf("  + Customer %d is now waiting.\n", id);

    // Wake up the barber if they're asleep
    if (isSleeping) sem_post(&custReady);
    pthread_mutex_unlock(&accessWRSeats);

    // Wait until the barber is ready
    sem_wait(&barberReady);

    printf("  > Customer %d getting haircut.\n", id);

    // Wait for haircut to be done
    sem_wait(&doneCutting);
    printf("  < Customer %d done.\n", id);
  } else {
    // If no free seats, leave
    pthread_mutex_unlock(&accessWRSeats);
    printf("  X Customer %d has no place to wait.\n", id);
  }
}

int main(int argc, char** argv) {
  // Barber begins ready
  sem_init(&barberReady, 0, 1);
  // Everything else set to 0
  sem_init(&custReady, 0, 0);
  sem_init(&doneCutting, 0, 0);

  // Create threads for the barber and the customers
  pthread_t barber_thread;
  pthread_create(&barber_thread, NULL, barber, NULL);
  pthread_t customer_threads[NUM_CUSTOMERS];
  for (int i = 0; i < NUM_CUSTOMERS; i++)
    pthread_create(&customer_threads[i], NULL, customer, (void*)i);

  // Wait for the threads to finish
  pthread_join(barber_thread, NULL);
  for (int i = 0; i < NUM_CUSTOMERS; i++)
    pthread_join(customer_threads[i], NULL);

  // Clean up
  sem_destroy(&barberReady);
  sem_destroy(&custReady);
  sem_destroy(&doneCutting);
  pthread_mutex_destroy(&accessWRSeats);

  return 0;
}