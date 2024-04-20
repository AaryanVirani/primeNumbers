# MPI Prime Number Generator

## Introduction

This MPI-based program generates prime numbers between 2 and a specified number N (limit is 10,000,000), leveraging parallel computing for improved scalability and performance. The goal is to demonstrate the effectiveness of distributed computing in numerical algorithms through the practical application of generating primes.

## Features

- **Distributed Computing:** Uses MPI to distribute the task of finding prime numbers across multiple processes.
- **Efficient Algorithm:** Implements a parallel version of the Sieve of Eratosthenes for efficient prime number generation.
- **Scalability Testing:** Designed to test and evaluate performance across different numbers of processors.

## How to Run the Program

### Prerequisites

- An MPI library (e.g., MPI) installed on your system.
- Access to a C compiler capable of compiling MPI programs (e.g., `mpicc`).
- Install the module with:
```bash
mpicc -o primes primes.c module load mpi/openmpi-x86_64
```

### Compilation

To compile the program, navigate to the directory containing `primes.c` and run the following command:

```bash
mpicc -o primes primes.c
