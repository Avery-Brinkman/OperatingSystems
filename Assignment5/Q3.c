#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Number of students that will try and meet with the TA
#define NUM_STUDENTS 10
// Number of chairs in the hallway
#define NUM_CHAIRS 3

// Mutex for getting number of free chairs
pthread_mutex_t hallwayMutex = PTHREAD_MUTEX_INITIALIZER;

// Notifies that the TA is ready
sem_t taReady;
// Notifies TA that a customer is waiting
sem_t studentReady;
// Notifies the student getting help that the TA is done helping
sem_t doneHelping;

// Number of free chairs for students
int freeChairs = NUM_CHAIRS;
// Whether or not the TA is napping
int isNapping = 0;

void* ta(void* arg) {
  while (1) {
    // Check if the hallway is empty
    pthread_mutex_lock(&hallwayMutex);
    if (freeChairs == NUM_CHAIRS) {
      // If empty, nap
      isNapping = 1;
      pthread_mutex_unlock(&hallwayMutex);

      // Wait for student
      printf("TA napping... ZZZzzz...\n");
      sem_wait(&studentReady);
    } else {
      // Update hallway chair count.
      freeChairs += 1;
      // Set TA to be awake
      isNapping = 0;
      pthread_mutex_unlock(&hallwayMutex);

      // Help a student for a random amount of time
      printf("Helping student.\n");
      sleep(rand() % 5 + 1);

      // Notify the student that the TA is done
      printf("Done helping student.\n");
      sem_post(&doneHelping);

      // Notify waiting students that the TA is ready.
      sem_post(&taReady);
    }
  }
}

void* student(void* arg) {
  int id = (int*)arg;
  while (1) {
    // Come to TA after random amount of time
    sleep(rand() % 10 + 1);

    // Check for free chairs
    pthread_mutex_lock(&hallwayMutex);
    if (freeChairs > 0) {
      // Take a chair and wait
      freeChairs -= 1;
      printf("  + Student %d is now waiting.\n", id);

      // Wake up the TA if they are napping
      if (isNapping) sem_post(&studentReady);
      pthread_mutex_unlock(&hallwayMutex);

      // Wait until the TA is ready
      sem_wait(&taReady);

      printf("  > Student %d getting help from TA.\n", id);

      // Wait for TA to finish helping student
      sem_wait(&doneHelping);
      printf("  < Student %d done getting help from TA.\n", id);
      return;
    } else {
      // If no free seats, repeat
      pthread_mutex_unlock(&hallwayMutex);
      printf("  X Student %d has no place to wait, coming back later.\n", id);
    }
  }
}

int main(int argc, char** argv) {
  // TA begins ready
  sem_init(&taReady, 0, 1);
  // Everything else set to 0
  sem_init(&studentReady, 0, 0);
  sem_init(&doneHelping, 0, 0);

  // Create threads for the TA and the students
  pthread_t barber_thread;
  pthread_create(&barber_thread, NULL, ta, NULL);
  pthread_t student_threads[NUM_STUDENTS];
  for (int i = 0; i < NUM_STUDENTS; i++)
    pthread_create(&student_threads[i], NULL, student, (void*)i);

  // Wait for the threads to finish
  pthread_join(barber_thread, NULL);
  for (int i = 0; i < NUM_STUDENTS; i++) pthread_join(student_threads[i], NULL);

  // Clean up
  sem_destroy(&taReady);
  sem_destroy(&studentReady);
  sem_destroy(&doneHelping);
  pthread_mutex_destroy(&hallwayMutex);

  return 0;
}