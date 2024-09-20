.. _nrf70_bm_porting_guide:

nRF70 Series BM driver Porting Guide
====================================
The nRF70 Series BM driver is designed to be portable across different platforms.
This guide provides an overview of the nRF70 Series BM library and the steps to port the library to a new platform.

nRF70 Series driver threading model
***********************************

The nRF70 Series BM driver uses a simple threading model to interact with the nRF70 Series device.
The driver code executes in the following contexts:"

* *Interrupt context*: For handling interrupts from the nRF70 Series. Interrupt service routines are used to schedule tasklets to offload the nRF70 Series event handling.
  The nRF70 device requires a single  `host IRQ` interrupt line to raise interrupts on the host platform, when the device needs to report an event. A GPIO pin needs to be configured as a `host IRQ` at the host MCU.
  The interrupt service routine reads the event coming from the nRF70 Series device and schedules a tasklet to handle the event.
* *Tasklet context*: Tasklets perform the actual work of interacting with the nRF70 Series device
  - processing events coming from the nRF70 device
  - writing and submitting commands to the nRF70 device
  RX tasklet: The RX tasklet reads the event data from the nRF70 Series and hands over to the FMAC callbacks.
* *Application thread context*: The application thread context is responsible for calling the nRF70 Series BM driver APIs to interact with the nRF70 Series device.

Zephyr shim reference
**********************


The `the nRF70 Series_zephyr_shim` directory contains a reference implementation for Zephyr.
This reference implementation can be used as a guide to port the library to other platforms.
This reference implementation is structured in multiple source directories as follows :

.. code-block:: none

    the nRF70 Series_zephyr_shim/     # Zephyr shim for the nRF70 Series, reference for third-party host platforms
      include/             # Include directory
      source/              # Source directory
        bus/               # Bus interface
        os/                # OS shim
        platform/          # Platform specific files
      CMakeLists.txt       # CMake build file```

The key components of the Zephyr shim reference implementation are:

* *os*: Contains the OS shim implementation for Zephyr.
* *bus*: Contains the bus interface implementation for SPI.
* *platform*: Contains the platform specific files, and has an RPU (Radio Processing Unit) abstraction layer that interacts with the nRF70 Series
  either through QSPI or SPI. The platform also uses Zephyr GPIO APIs to manage GPIO pins of the nRF70 Series.

Zephyr functionality used in the reference implementation:
**********************************************************

The reference implementation of the BM Driver for the Zephyr RTOS uses build-time tools as well as standard OS primitives; these elements require porting effort when integrating the driver to a third-party (non-Zephyr) platform.

* OS primitives: The reference implementation uses the following Zephyr OS primitives:

    - Workqueue: Used to schedule work or delayed work to be done.
    - Heap: Used to allocate memory for the nRF70 Series.
    - Lists: Used to manage the list of tasks to be done in the context of the nRF70 Series.
    - Sleep/Delay: Used to manage the sleep and delay for the nRF70 Series.
    - Semaphore: Used to manage the synchronization between the nRF70 Series and the host MCU.
* Driver model: The reference implementation uses the Zephyr driver model to manage the nRF70 Series device.

    - SPI: Uses Zephyr's SPI driver to communicate with the nRF70 Series over SPI.
    - GPIO: Used to manage the host IRQ GPIO pin of the nRF70 Series.

* Build time tools: The reference implementation uses the following build-time tools:

    - DTS: Used to define the GPIO pins of the nRF70 Series.
    - Kconfig: Used to define the configuration options for the nRF70 Series driver.
