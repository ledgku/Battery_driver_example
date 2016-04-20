# Battery_driver_example
device driver(kernel) &amp; power manager(user) system programming example (Using raspberrypi)

## Getting started
```sh
$ make
$ sudo insmod battery_module.ko
$ gcc power_manager.c -o power_manager -pthread
$ sudo ./power_manager
```
