.. _nrf70_bm_porting_guide:

nRF70 Series BM driver Porting Guide
####################################

The nRF70 Series BM driver is designed to be portable across different platforms.
This guide provides an overview of the steps required to port the library to a new platform or OS environment.


Zephyr functionality used in the reference implementation:
**********************************************************

The reference implementation of the BM Driver for the Zephyr RTOS uses build-time tools as well as standard OS primitives; these elements require porting effort when integrating the driver to a third-party (non-Zephyr) platform.

* *OS primitives*: The reference implementation uses the following Zephyr OS primitives:

    - Workqueue: Used to schedule work or delayed work to be done.
    - Heap: Used to allocate memory for the nRF70 Series.
    - Lists: Used to manage the list of tasks to be done in the context of the nRF70 Series.
    - Sleep/Delay: Used to manage the sleep and delay for the nRF70 Series.
    - Semaphore: Used to manage the synchronization between the nRF70 Series and the host MCU.
* *Driver model*: The reference implementation uses the Zephyr driver model to manage the nRF70 Series device.

    - SPI: Uses Zephyr's SPI driver to communicate with the nRF70 Series over SPI.
    - GPIO: Used to manage the host IRQ GPIO pin of the nRF70 Series.

* *Build time tools*: The reference implementation uses the following build-time tools:

    - DTS: Used to define the GPIO pins of the nRF70 Series.
    - Kconfig: Used to define the configuration options for the nRF70 Series driver.
