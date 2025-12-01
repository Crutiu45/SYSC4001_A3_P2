# How to Run C files

Navigate into the "src" directory of Part2_Concurrency folder

# Compile C files

Compile ta_marking.c file: gcc -o ta_marking ta_marking.c helpers.c

Compile ta_marking_sync file: gcc -o ta_marking_sync ta_marking_sync.c helpers.c

# Run the C files

Run ta_marking.c file: ./ta_marking 3

Run ta_marking_sync file: ./ta_marking_sync 3

# Test Cases
To run different test cases change the number of TA processes 

1. Run with 2 TAs 
./ta_marking 2 ,  ./ta_marking_sync 2

2. Run with 3 TAs 
./ta_marking 3 ,  ./ta_marking_sync 3


3. Run with 5 TAs 
./ta_marking 5 ,  ./ta_marking_sync 5


