## UID: 506053914

## Pipe Up

Low-level C program replicating the shell pipe operator through the use of manually manipulating file descriptors.

## Building

To build this program, run the following command (make sure your current directory is the folder with the pipe.c and Makefile).

```bash
make
```

## Running

Example run of program with two additional arguments.
Expected output is shown with the original commands before its respective ./pipe equivalent below.

```bash
rolandyang@Rolands-MacBook-Pro lab1 % ls
Makefile        README.md       pipe            pipe.c          pipe.o          test_lab1.py
rolandyang@Rolands-MacBook-Pro lab1 % ./pipe ls
Makefile        README.md       pipe            pipe.c          pipe.o          test_lab1.py
rolandyang@Rolands-MacBook-Pro lab1 % ls       
Makefile        README.md       pipe            pipe.c          pipe.o          test_lab1.py
rolandyang@Rolands-MacBook-Pro lab1 % ./pipe ls cat wc
       6       6      51
rolandyang@Rolands-MacBook-Pro lab1 % ls | cat | wc
       6       6      51
```

## Cleaning up

To clean up all binary files, run the following command.

```bash
make clean
```
