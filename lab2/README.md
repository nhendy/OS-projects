# Lab 2 instructions:

## NOTES about lab2/apps/Makefile modifications:

In order to support linking binaries that act as libraries two variables were added:

`INTERNAL_LIBS_DIR`: This variable is specified at each question's directory Makerules (e.g: `lab2/apps/q2/Makerules`) and is used to identify path of where
the utility libraries are located

`INTERNAL_LIBS_OBJS`: This is variable is specified at each binary target Makefile (e.g: `lab2/apps/q2/makeprocs/Makefile`) and is used to identify which library from `INTERNAL_LIBS_DIR`
is needed and where it will be compiled (right not it'll be compiled in the work/ folder where all other source files are compiled)


## Question 2 instructions:
```bash
cd lab2/os && make clean && make
cd -
cd lab2/apps/q2 && make clean && make
```
## Question 4 instructions:
```bash
cd lab2/os && make clean && make
cd -
cd lab2/apps/q4 && make clean && make
```

## Question 5 instructions:
```bash
cd lab2/os && make clean && make
cd -
cd lab2/apps/q5 && make clean && make
```
