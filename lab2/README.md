# You Spin Me Round Robin

C implementation of the Round Robin Scheduling algorithm, along with a dynamic quantum length algorithm based off the medium of CPU time usage of all processes in the queue at the time.

## Building

```shell
make
```

## Running

Populate a text file in the appropriate format then run the commands below. Sample output for an example provided.

processes.txt:
```shell
4
1, 0, 7
2, 2, 4
3, 4, 1
4, 5, 4
```

```shell
./rr processes.txt 3
```

Results:
```shell
Average waiting time: 7.00
Average response time: 2.75
```

## Cleaning up

```shell
make clean
```
