# How to Run C files

Navigate into the "src" directory of Part2_Concurrency folder

# Compile C files

Compile ta_marking.c file: gcc -o ta_marking_101195158_101294948 ta_marking_101195158_101294948.c helpers.c

Compile ta_marking_sync file: gcc -o ta_marking_sync_101195158_101294948 ta_marking_sync_101195158_101294948.c helpers.c

# Run the C files

Run ta_marking.c file: ./ta_marking_101195158_101294948 3

Run ta_marking_sync file: ./ta_marking_sync_101195158_101294948 3

# Test Cases
To run different test cases change the number of TA processes 

1. Run with 2 TAs 
./ta_marking_101195158_101294948 2 ,  ./ta_marking_sync_101195158_101294948 2

2. Run with 3 TAs 
./ta_marking_101195158_101294948 3 ,  ./ta_marking_sync_101195158_101294948 3


3. Run with 5 TAs 
./ta_marking_101195158_101294948 5 ,  ./ta_marking_sync_101195158_101294948 5


