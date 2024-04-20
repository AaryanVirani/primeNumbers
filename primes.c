#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    if(argc != 2){
        printf("Usage: mpirun -np <num_processes> ./primeNumbers <N>\n");
        return 1;
    }

    // Initialize MPI
    int my_rank, comm_sz;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    // Check time taken to execute the program
    double start_time, end_time;
    if (my_rank == 0){
        start_time = MPI_Wtime();
    }

    // Get N from command line arguments 
    int N = atoi(argv[1]);

    // Condition to check if N < 2 or greater than 1000000
    if (N < 2 || N > 10000000){
        if (my_rank == 0){
            printf("N should be between 2 and 1000000\n");
        }
        MPI_Finalize();
        return 1;
    }

    // Calculate the range of numbers to be checked by each process
    int range = N - 1;
    int num_per_process = range / comm_sz;
    int remainder = range % comm_sz;

    // Calculate the start and end values for each process
    int start = my_rank * num_per_process + 2; // Start from 2
    int end = start + num_per_process - 1; // Due to inclusive range

    // Distribute the remainder, one for each process, until it's used up
    if (my_rank < remainder) {
        start += my_rank;
        end += my_rank + 1;
    } else {
        start += remainder;
        end += remainder;
    }

    // Correcting the end value for the last process
    if (my_rank == comm_sz - 1){
        end = N;
    }
    
    // Initialize a local array to sieve the prime numbers
    int* local_primes = (int*) malloc((end - start + 1) * sizeof(int));
    for (int i = 0; i < end - start + 1; i++){
        local_primes[i] = 1; // Assume all numbers are prime
    }

    // Calculate the square root of N for the sieve
    double sqrt_N = sqrt(N);

    // Sieve the prime numbers
    for (int i = 2; i <= sqrt_N; i++){
        int is_prime;
        // Root process checks if i is prime and broadcasts the result
        if (my_rank == 0){
            is_prime = 1;
            for (int j = 2; j <= sqrt(i); j++){
                if (i % j == 0){
                    is_prime = 0;
                    break;
                }
            }
        }
        // Broadcast the result to all processes to handle multiples of i
        MPI_Bcast(&is_prime, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (is_prime){
            // Calculate the first multiple of i in the range
            // If start is beyond i*i (since multiples before i*i are already considered), 
            // find the nearest multiple of i above 'start'. Otherwise, begin from i*i.
            int first_multiple = (start > i * i) ? start + ((i - (start % i))% i): i * i;

            // Mark all multiples of i as non-prime 
            for (int j = first_multiple; j <= end; j += i){
                if (j >= start)
                    local_primes[j - start] = 0;
            }
        }
    }

    // Check time and print primes before outputting to .txt file
    if (my_rank == 0){
        end_time = MPI_Wtime();
        printf("Time taken: %f seconds\n", end_time - start_time);
    }

    // Gather the prime numbers from all processes and print it to a .txt file 
    if (my_rank == 0){
        // Create a file with the name as N.txt
        char filename[20];
        sprintf(filename, "%d.txt", N);
        FILE* fptr = fopen(filename, "w");

        // Print the prime numbers from the root process first since they contain the first few prime numbers
        for (int i = 0; i < end - start + 1; i++){
            if (local_primes[i] == 1){
                fprintf(fptr, "%d ", i + start);
            }
        }

        // Receive the prime numbers from all other processes in order of rank (except rank 0 - root process)
        for (int i = 1; i < comm_sz; i++){
            int num_primes; // Number of prime numbers from each process
            MPI_Recv(&num_primes, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // Receive the number of prime numbers
            int* primes = (int*) malloc(num_primes * sizeof(int)); // Allocate memory to store the prime numbers
            MPI_Recv(primes, num_primes, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // Receive the prime numbers and put it in the array
            for (int j = 0; j < num_primes; j++){ // Print the prime numbers to the file
                fprintf(fptr, "%d ", primes[j]);
            }
            free(primes);
        }
        fclose(fptr);
    } else {
        // Send the prime numbers to the root process
        int num_primes = 0;
        for (int i = 0; i < end - start + 1; i++){
            if (local_primes[i] == 1){
                num_primes++;
            }
        }
        MPI_Send(&num_primes, 1, MPI_INT, 0, 0, MPI_COMM_WORLD); // Send the number of prime numbers
        int* primes = (int*) malloc(num_primes * sizeof(int)); // Allocate memory to store the prime numbers
        int index = 0;
        for (int i = 0; i < end - start + 1; i++){ // Store the prime numbers
            if (local_primes[i] == 1){
                primes[index] = i + start;
                index++;
            }
        }
        MPI_Send(primes, num_primes, MPI_INT, 0, 0, MPI_COMM_WORLD); // Send the prime numbers
        free(primes);
    }
    
    // Freeing the local_primes array
    free(local_primes);
    
    // Finalize MPI
    MPI_Finalize();

    return 0;
}
