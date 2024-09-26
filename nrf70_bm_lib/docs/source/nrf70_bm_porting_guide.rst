.. _nrf70_bm_porting_guide:

nRF70 Series BM driver Porting Guide
####################################

The nRF70 Series BM driver is designed to be portable across different platforms.
This guide provides an overview of the steps required to port the library to a new platform or OS environment.


Zephyr functionality used in the reference implementation:
**********************************************************

The reference implementation of the BM Driver for the Zephyr RTOS uses build-time tools as well as standard OS primitives; these elements require porting effort when integrating the driver to a third-party (non-Zephyr) platform.

.. list-table:: OS primitives porting guidelines
  :header-rows: 1

  * - OS primitive
    - Description
    - OS agnostic layer API
    - Reference Zephyr API
  * - Tasklet
    - Used to process offloaded tasks from the nRF70 ISR context  (typically events coming from the nRF70 Series device)
    - tasklet_schedule()
    - k_work_submit()
  * - Heap
    - Used to allocate memory for the nRF70 Series driver.
    - mem_alloc()
    - k_malloc()
  * - Lists
    - Used to manage the list of tasks to be done in the context of the nRF70 Series.
    - llist_***
    - sys_slist_***
  * - Sleep/Delay
    - Used to manage the sleep and delay for the nRF70 Series.
    - sleep_ms()/delay_us()
    - k_msleep()/k_usleep()
  * - Spinlocks
    - Used to manage the synchronization between the nRF70 Series and the host MCU.
    - spinlock_***
    - k_sem_***
  * - Timers
    - Used to manage the timers for the nRF70 Series driver, esp. for low power mode.
    - timer_***
    - k_work_delayable_***


.. note ::

   The synchronization primitives used in the latest reference implementation have been updated to use Zephyr spinlocks instead of semaphores.

* *Driver model*: The reference implementation uses the Zephyr driver model to manage the nRF70 Series device.

    .. list-table:: Driver model porting guidelines
      :header-rows: 1

      * - Driver model component
        - Description
        - OS agnostic layer API
        - Reference Zephyr API
      * - SPI
        - Uses Zephyr's SPI driver to communicate with the nRF70 Series over SPI.
        - qspi_*_reg32()/qspi_cpy_*()
        - spi_transceive_dt()
      * - GPIO
        - Uses Zephyr's GPIO driver to manage the GPIO pins of the nRF70 Series.
        - bus_qspi_dev_add()/bus_qspi_dev_rem()
        - gpio_pin_configure_dt()

* *Build time tools*: The reference implementation uses the following build-time tools:

    - DTS: Used to define the GPIO configuration for the nRF70 Series on the host platform

        - the nRF70 Series BM driver uses C headers over DTS for any hardware-specific configuration

    - Kconfig: Used to define the configuration options for the nRF70 Series driver

        - Zephyr's build system generates autoconf.h based on the Kconfig options, this can be used as a basis for the third-party platform.

OS agnostic driver layer
************************

The BM library uses the OS-agnostic nRF70 Wi-Fi driver layer to interact with the nRF70 Series device.
This layer is pulled in as a submodule from the nRF Connect SDK nrfxlib repository using a custom branch.

For more information, visit the `nRF Connect SDK nrfxlib repository <https://github.com/nrfconnect/sdk-nrfxlib>`_.

.. note ::

   The git submodule pulls in the entire repo, but only `nrf_wifi` directory is required for the BM driver.
