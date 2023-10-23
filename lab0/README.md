# A Kernel Seedling
The following documents lab0 for CS111. A module that counts how many processes are active in a kernel.
506053914 Roland Yang

## Building
```shell
make
```

## Running
```shell
sudo insmod proc_count.ko
cat /proc/count
```
Upon running this on my VM, my /proc/count contained the singular integer "138".

## Cleaning Up
```shell
sudo rmmod proc_count
make clean
```

## Testing
```python
python -m unittest
```
The results of running the unit tests outputted:
```shell
Ran 3 tests in 5.960s

OK
```

Report which kernel release version you tested your module on
(hint: use `uname`, check for options with `man uname`).
It should match release numbers as seen on https://www.kernel.org/.

```shell
uname -r -s -v
```
The kernel version of my Linux VM is "5.14.8-arch-1"