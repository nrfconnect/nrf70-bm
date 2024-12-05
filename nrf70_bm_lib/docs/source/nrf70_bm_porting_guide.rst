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
  * - IRQ
    - Used to manage the interrupts from the nRF70 Series device.
    - nrf_wifi_osal_bus_qspi_dev_intr_reg()/nrf_wifi_osal_bus_qspi_dev_intr_unreg()
    - gpio_add_callback()/gpio_remove_callback()
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
  * - Pseudo-Random Number Generator (PRNG)
    - Used to generate random numbers for the nRF70 Series random MAC address generation,
      (if random MAC address generation support is enabled).
    - NA
    - sys_rand8_get()


.. note ::

   The synchronization primitives used in the latest reference implementation have been updated to use Zephyr spinlocks instead of semaphores.

.. note ::

   The IRQ for nRF70 Series is configued as a GPIO pin, and is Edge triggered i.e., interrupt to be triggered on pin state change to logical level 1.

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

* *Build time tools*: The reference implementation uses the following build-time tools, see also `Zephyr build auto-generated files` section below.

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

Transmit (TX) power configuration
*********************************

The maximum transmit output power achieved on an nRF70 Series device-based product depends on the frequency band and operating channel.
This varies across different Printed Circuit Board (PCB) designs.

Multiple calibrations and checks are implemented to ensure consistent TX output power across channels and devices.
However, as the resulting TX output power depends on PCB design; Error Vector Magnitude (EVM) and spectral mask failures may occur, unless the TX output power ceilings are properly applied.
The application developer can specify the TX power ceiling at which the EVM and spectral mask compliance is met for a given PCB design.

The build-time parameters for the TX power ceilings are made available to nRF Wi-Fi driver through the respective `C` header file.
The reference header files are located in the `nrf70_bm_lib/include` directory.

The following files are used to configure the TX power ceilings for the official Nordic development and evaluation boards:

- `nrf70_tx_pwr_ceil_dk.h`: Contains the TX power configuration for the nRF7002 Development Kit (DK).

- `nrf70_tx_pwr_ceil_ek.h`: Contains the TX power configuration for the nRF7002 Evaluation Kit (EK).

For a new nRF70-based PCB design, the developer may create a new header file with the same structure as the reference files and replace the values with the appropriate values for the designed PCB.
The appropriate TX power ceiling values may be derived by running the radio test sample on the board and using a spectrum/vector signal analyzer to measure the output power, spectral mask, EVM and other parameters and then adjusting the TX power ceiling values to meet the requirements.
The values are represented in 0.25 dB increments.

For more information on TX power calculation, refer to the section `nRF70 TX power calculation <https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/drivers/wifi/nrf70_native.html#tx_power_calculation>`_.


Reference header file structure
-------------------------------

.. doxygenfile:: nrf70_tx_pwr_ceil_dk.h


nRF70 OS agnostic layers API documentation
==========================================

The nRF70 Series OS agnostic layers are documented along with the APIs.
The below modules are part of the nRF70 Series OS agnostic layer:


.. toctree::
  :maxdepth: 1

  nrf70_osal_doc


Zephyr build auto-generated files
*********************************

The Zephyr build system generates the following auto-generated files that are used by the nRF70 Series BM driver,
the files are located in the build directory of the Zephyr build system, typically in the ``build/zephyr/`` directory.

- ``include/generated/autoconf.h``: Contains the configuration options defined in Kconfig
- ``include/generated/devicetree_generated.h``: Contains the Bus, GPIO configuration defined in the DTS file

These can be used as a reference for the third-party platform to define the configuration options and hardware-specific configuration.
