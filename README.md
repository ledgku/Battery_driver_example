# Battery_driver_example
device driver(kernel) &amp; power manager(user) system programming example (Using raspberrypi)

## Getting started
```sh
$ make
$ sudo insmod battery_module.ko
$ gcc power_manager.c -o power_manager -pthread
$ sudo ./power_manager
$ ./simulate.sh
```
## Flow
![flow](https://cloud.githubusercontent.com/assets/12126093/14732091/0a33a298-0892-11e6-96a6-38e534ee6fc4.png)
