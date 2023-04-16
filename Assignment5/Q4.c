#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Number of elements to store in the vector
#define NUM_ELEMENTS 5
// Number of threads that want to write data
#define NUM_READERS 10
// Number of threads that want to read data
#define NUM_WRITERS 10

// Mutex to keep data safe
pthread_mutex_t dataMutex = PTHREAD_MUTEX_INITIALIZER;

// The data
int data[NUM_ELEMENTS];

void* readData(void* arg) {
  int id = (int)arg;

  // Try to read data after random amount of time
  sleep(rand() % 10);

  // Try to read a random element from the array
  int dataIndex = rand() % NUM_ELEMENTS;
  printf("  R%d waiting to read %d\n", id, dataIndex);

  // Try and lock the mutex, putting this thread next in line to access once
  // it's freed
  pthread_mutex_lock(&dataMutex);

  // Take a random amount of time to read data
  printf("R%d reading...\n", id);
  sleep(rand() % 2 + 1);
  printf("[%d] => %d\n", dataIndex, data[dataIndex]);

  // Release memory for other threads
  pthread_mutex_unlock(&dataMutex);
}

void* writeData(void* arg) {
  int id = (int)arg;

  // Try to write data after random amount of time
  sleep(rand() % 10);

  // Try to set a random element from the array to a random value
  int dataIndex = rand() % NUM_ELEMENTS;
  int newValue = rand() % 100;
  printf("  W%d waiting to set [%d] = %d\n", id, dataIndex, newValue);

  // Try and lock the mutex, putting this thread next in line to access once
  // it's freed
  pthread_mutex_lock(&dataMutex);

  // Take a random amount of time to write data
  printf("W%d writing...\n", id);
  data[dataIndex] = newValue;
  sleep(rand() % 2 + 1);
  printf("[%d] <= %d\n", dataIndex, newValue);

  // Release memory for other threads
  pthread_mutex_unlock(&dataMutex);
}

int main() {
  // Initialize data to 0s
  for (int i = 0; i < NUM_ELEMENTS; i++) data[i] = 0;

  // Create and initialize threads for reading and writing the data
  pthread_t readThreads[NUM_READERS];
  pthread_t writeThreads[NUM_WRITERS];
  for (int i = 0; i < NUM_READERS; i++)
    pthread_create(&readThreads[i], NULL, readData, (void*)i);
  for (int i = 0; i < NUM_WRITERS; i++)
    pthread_create(&writeThreads[i], NULL, writeData, (void*)i);

  // Wait for the threads to finish
  for (int i = 0; i < NUM_READERS; i++) pthread_join(readThreads[i], NULL);
  for (int i = 0; i < NUM_WRITERS; i++) pthread_join(writeThreads[i], NULL);

  // Clean up
  pthread_mutex_destroy(&dataMutex);

  printf("Final results: ");
  for (int i = 0; i < NUM_ELEMENTS; i++) printf("%d ", data[i]);
  printf("\n");
}