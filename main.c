#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 3

// initializes all matricies needed for bankers algorithm including copies

int avail[NUMBER_OF_RESOURCES];
int max[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int alloc[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

int availCopy[NUMBER_OF_RESOURCES];
int maxCopy[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int allocCopy[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int needCopy[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

int finish[NUMBER_OF_CUSTOMERS] = {0};
int safeSequence[NUMBER_OF_CUSTOMERS] = {0};

int release_resources(int customerNum);
int request_resources(int customerNum, int request[]);
void *thread_func(void *customerNumt);
int bankerAlgorithm(int customerNum, int request[]);
void printState();
pthread_mutex_t mutex;

int main(int argc, char const *argv[])
{

    // Checks that value has been entered for all resources
    if (argc != NUMBER_OF_RESOURCES + 1)
    {

        fprintf(stderr, "Invalid input please input a value for all resources.\n");
        exit(1);
    }

    // Checks that value for each resource is non-negative
    for (int i = 1; i < argc; i++)
    {

        if (atoi(argv[i]) < 0)
        {

            fprintf(stderr, "Invalid value entered at resource %d, resources may not be negative values.\n", i);
            exit(1);
        }
    }

    //------------------Matrix Initialization-------------------
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++)
    {

        avail[i] = atoi(argv[i + 1]);

        for (int j = 0; j < NUMBER_OF_CUSTOMERS; j++)
        {
            // Sets maximum value to random value that is not greater than available resources
            max[j][i] = rand() % (avail[i] + 1);

            // Need is equal to max
            need[j][i] = max[j][i];

            // allocated defaults to 0
            alloc[j][i] = 0;
        }
    }
    //---------------------------------------------------------
    printState();

    pthread_mutex_init(&mutex, NULL);

    // IMPORTANT: this array must be changed to match number of customers (has been left at 5 for exxample purposes)
    int processNum[] = {0, 1, 2, 3, 4, 5};

    pthread_t p[NUMBER_OF_CUSTOMERS];

    // Creates all processes
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
    {
        printf("%d\n", i);
        pthread_create(&(p[i]), NULL, thread_func, &processNum[i]);
    }

    char *returnVal;

    // Wyaits for each process to finish before completing
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
    {
        pthread_join((p[i]), (void **)&returnVal);
    }

    printf("All Processes completed successfully.\n");
    return 0;
    exit(1);
}

void *thread_func(void *tempCustomerNum)
{
    int *c = (int *)tempCustomerNum;
    int customerNum = *c;

    int sumRequest = 0;

    // loops until thread finishes and need for said thread is 0
    while (!finish[customerNum])
    {
        sumRequest = 0;
        int request[NUMBER_OF_RESOURCES] = {0};

        for (int i = 0; i < NUMBER_OF_RESOURCES; i++)
        {
            // generates random request below process need
            request[i] = rand() % (need[customerNum][i] + 1);
            sumRequest = sumRequest + request[i];
        }

        // makes sure process does not request 0 resources
        if (sumRequest != 0)
            // loops until request is successfully granted then terminates
            while (request_resources(customerNum, request) == -1)
                ;
    }

    return 0;
}

int request_resources(int customerNum, int request[])
{

    int returnVal = -1;

    // locks mutex
    pthread_mutex_lock(&mutex);

    printf("\nP%d requests:", customerNum + 1);

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++)
    {
        printf("%d ", request[i]);
    }
    printf("\n");

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++)
    {
        // checks that request <= available otherwise returns -1
        if (request[i] > avail[i])
        {
            printf("P%d waiting for resources...\n", customerNum + 1);

            // unlocks mutex before returning thread
            pthread_mutex_unlock(&mutex);
            return -1;
        }
    }

    // executes banker's algorithm
    returnVal = bankerAlgorithm(customerNum, request);

    if (returnVal == 0)
    {
        int needIsZero = 1;
        printf("Safe sequence found: ");
        for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
        {
            printf("P%d ", safeSequence[i] + 1);
        }
        printf("\nP%d resource request granted\n", customerNum + 1);

        for (int j = 0; j < NUMBER_OF_RESOURCES; j++)
        { // give the resources to the theread
            alloc[customerNum][j] = alloc[customerNum][j] + request[j];
            avail[j] = avail[j] - request[j];
            need[customerNum][j] = need[customerNum][j] - request[j];
            if (need[customerNum][j] != 0)
            { // to check if need is zero
                needIsZero = 0;
            }
        }

        if (needIsZero)
        {
            // marks thread as finished when need reaches 0 and releases all resources for the thread
            finish[customerNum] = 1;
            release_resources(customerNum);
        }

        printState();
    }
    else
    {
        printf("WARNING: safe sequence not found\n");
    }

    // unlocks mutex
    pthread_mutex_unlock(&mutex);
    return returnVal;
}

// Function used to release resources
int release_resources(int customerNum)
{

    printf("P%d has released all resources\n", customerNum + 1);
    for (int j = 0; j < NUMBER_OF_RESOURCES; j++)
    {
        avail[j] = avail[j] + alloc[customerNum][j];
        alloc[customerNum][j] = 0;
    }

    return 0;
}

int bankerAlgorithm(int customerNum, int request[])
{
    int finish[NUMBER_OF_CUSTOMERS] = {0};

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++)
    {
        // Copies all matricies for safety testing
        availCopy[i] = avail[i];
        for (int j = 0; j < NUMBER_OF_CUSTOMERS; j++)
        {
            allocCopy[j][i] = alloc[j][i];

            maxCopy[j][i] = max[j][i];

            needCopy[j][i] = need[j][i];
        }
    }

    // pretends to pass copied matricies to thread
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++)
    {
        availCopy[i] = availCopy[i] - request[i];
        allocCopy[customerNum][i] = allocCopy[customerNum][i] + request[i];
        needCopy[customerNum][i] = needCopy[customerNum][i] - request[i];
    }

    //---------------------Safety Algorithm---------------
    int count = 0;
    while (1)
    {

        int I = -1;

        for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
        {
            // Finds thread thats need is equal to or less than available resources
            int nLessThanA = 1;
            for (int j = 0; j < NUMBER_OF_RESOURCES; j++)
            {
                if (needCopy[i][j] > availCopy[j] || finish[i] == 1)
                {
                    nLessThanA = 0;
                    break;
                }
            }
            if (nLessThanA)
            {
                // records number of found thread
                I = i;
                break;
            }
        }

        if (I != -1)
        {
            //records sequence
            safeSequence[count] = I;
            count++;
            finish[I] = 1; // mark the thread "finish"
            for (int k = 0; k < NUMBER_OF_RESOURCES; k++)
            { // pretend to give the reaource to thread
                availCopy[k] = availCopy[k] + allocCopy[I][k];
            }
        }
        else
        {
            for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
            {
                if (finish[i] == 0)
                {
                    // if a thread can't be found there is no safe sequence. -1 is returned
                    return -1;
                }
            }
            // returns when all friends are found
            return 0;
        }
    }
    //----------------------------------------------------
}

// function responsible for printing current state of resources
void printState()
{

    printf("Allocated:\n\n");
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
    {
        printf("P%d ", i + 1);
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++)
        {
            printf("%d ", alloc[i][j]);
        }
        printf("\n");
    }
    printf("\nNeeded:\n\n");
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
    {
        printf("P%d ", i + 1);
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++)
        {
            printf("%d ", need[i][j]);
        }

        printf("\n");
    }
    printf("\nAvailable:\n\n");
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++)
    {
        printf("%d ", avail[i]);
    }

    printf("\n");
}
