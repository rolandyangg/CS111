# Hey! I'm Filing Here

## Roland Yang 506053914

This lab successfully implements a mountable ext2 file system image containing a root directory, lost+found directory, a regular file named hello-world, and a symbolic linked name hello which points to hello-world.

## Building

Run the following command in the directory of all the files to compile the executable.
```shell
make
```

This will create an exectuable called `ext2-create` that we can use to create our file system image. Run the executable.

```shell
./ext2-create
```

Upon successful execution we should have our image `cs111-base.img`

## Running

We can do several things with our image. To dump the file system information run the following command.
```shell
dumpe2fs cs111-base.img
```

To check that the file system is correct run the following command.
```shell
fsck.ext2 cs111-base.img
```

To actually mount our file system, we must create a directory and run a command to mount it. The following commands below makes a temporary folder which we then mount our image onto.
```shell
mkdir mnt
sudo mount -o loop cs111-base.img mnt
```

## Cleaning up

Upon being finished with the file system we can unmount it from our temporary folder using the following command. We can also optionally remove the folder afterwards.
```shell
sudo umount mnt
rmdir mnt
```

To clean all other excess files such as the executable and image run the following command.
```shell
make clean
```