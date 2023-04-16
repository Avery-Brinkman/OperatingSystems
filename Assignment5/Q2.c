#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Set a limit to how many times it runs
#define MAX_ITERATIONS 5

// Synchronization
sem_t smokerSem[3];
sem_t agentSem;

// Shared memory
int ingredient1;
int ingredient2;

// For printing
const char* ingredientNames[3] = {"tobacco", "paper", "matches"};

void* agentThread(void* arg) {
  int iter = 0;
  while (iter < MAX_ITERATIONS) {
    // Wait for the signal to make a cigarette
    sem_wait(&agentSem);

    // Choose two random ingredients
    ingredient1 = rand() % 3;
    ingredient2 = rand() % 3;
    while (ingredient2 == ingredient1) ingredient2 = rand() % 3;

    printf("Agent placing %s and %s onto the table.\n",
           ingredientNames[ingredient1], ingredientNames[ingredient2]);

    // Signal the smokers
    for (int i = 0; i < 3; i++) sem_post(&smokerSem[i]);

    iter++;
  }
}

void* smokerThread(void* arg) {
  int ingredient = (int)(int*)arg;

  int iter = 0;
  while (iter < MAX_ITERATIONS) {
    iter++;

    // Wait for a signal from the relevant semaphore
    sem_wait(&smokerSem[ingredient]);

    // Check if the ingredients on the table are the ones needed
    if (ingredient == ingredient1 || ingredient == ingredient2) continue;

    // Take a random amount of time to make the cigarette
    printf("Smoker with %s is making a cigarette.\n",
           ingredientNames[ingredient]);
    sleep(rand() % 2 + 1);

    // Take a random amount of time to smoke the cigarette
    printf("Smoker is smoking the cigarette.\n");
    sleep(rand() % 2 + 1);

    // Signal the agent to provide more ingredients
    printf("Smoker is done.\n\n");
    sem_post(&agentSem);
  }
}

int main() {
  // Initialize semaphores
  for (int i = 0; i < 3; i++) sem_init(&smokerSem[i], 0, 0);
  // Agent starts out ready
  sem_init(&agentSem, 0, 1);

  // Create threads
  pthread_t agent_tid;
  pthread_t smoker_tids[3];
  pthread_create(&agent_tid, NULL, agentThread, NULL);
  for (int i = 0; i < 3; i++)
    pthread_create(&smoker_tids[i], NULL, smokerThread, (void*)i);

  // Join threads
  pthread_join(agent_tid, NULL);
  for (int i = 0; i < 3; i++) pthread_join(smoker_tids[i], NULL);

  // Clean up
  for (int i = 0; i < 3; i++) sem_destroy(&smokerSem[i]);
  sem_destroy(&agentSem);

  return 0;
}
