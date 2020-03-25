# Lab 3 instructions:

## NOTES about lab3/apps/Makefile modifications:

In order to support linking binaries that act as libraries two variables were added:

`INTERNAL_LIBS_DIR`: This variable is specified at each question's directory Makerules (e.g: `lab3/apps/q2/Makerules`) and is used to identify path of where
the utility libraries are located

`INTERNAL_LIBS_OBJS`: This is variable is specified at each binary target Makefile (e.g: `lab3/apps/q2/makeprocs/Makefile`) and is used to identify which library from `INTERNAL_LIBS_DIR`
is needed and where it will be compiled (right not it'll be compiled in the work/ folder where all other source files are compiled)


## Question 2 instructions:
```bash
cd lab3/os && make clean && make
cd -
cd lab3/apps/q2 && make clean && make run
```
## Question 4 instructions:
```bash
cd lab3/os && make clean && make
cd -
cd lab3/apps/prio_test && make clean && make run > mylogs.txt
# Use your fav diff editor to compare mylogst.txt and expected_output
vimdiff mylogs.txt expected_output
```
NOTES on the diff:
Currently the diff is only at the top of the file where there's a few warnings and makefile logs
and the execution time.

## Question 5 instructions:
```bash
cd lab3/os && make clean && make
cd -
cd lab3/apps/q5 && make clean && make run
```

To test Q5 I copied the same program in prio_test and made even numbered programs sleep
and others yield.
You can see from the logs the scheduler in action.
When a process yields it says PID #29 yields for example
It also prints the runqueues for visualization.


The idle process by default has pinfo enabled. If you'd like to turn that off
you can modify ProcessForkIdle in process.c to the following


```cpp
void ProcessForkIdle() { ProcessFork(&ProcessIdle, 0, 0, 0, "idle", 0); }
```


There's also a test_mbox folder for light testing the mbox library corner cases.
