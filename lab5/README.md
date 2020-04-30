# Lab 5 Group 55 (nhendy, drijhwan)

## Instructions

### fdisk (Q1)

Running fdisk requires that either the file disk be invalid initially or that DFS module be disabled. So either `touch /tmp/ee469g55.img` then run it in the following way

```sh
cd flat/os && make
cd -
cd flat/apps/fdisk && make run 
```

Or you can use the provided `reformat_disk.sh` script which passes a flag `-x` to OS to
disable DFS module.

 ```sh
./reformat_disk.sh
```

It assumes that the disk image is `/tmp/ee469g55.img` and runs `chmod a+rwx` on it to make it usable by others


### Tests (os tests and file tests) (Q4 and Q6)

Assuming that file specified in `DISK_FILENAME` exists and formatted properly, you can simply run the script `run_tests.sh` or execute the commands it has manually if you'd like

```sh
./run_tests.sh
```
