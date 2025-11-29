(
echo # Part 2 - TA Marking Concurrency
echo.
echo ## Compilation
echo gcc -o ta_marking src/ta_marking.c
echo gcc -o ta_marking_sync src/ta_marking_sync.c
echo.
echo ## Execution
echo ./ta_marking 3
echo ./ta_marking_sync 3
echo.
echo Where 3 is the number of TAs (processes^)
) > README.md
