.. _nrf70_bm_porting_guide:

nRF70 Series BM driver Porting Guide
####################################

The nRF70 Series BM driver is designed to be portable across different platforms.
This guide provides an overview of the steps required to port the library to a new platform or OS environment.


Zephyr functionality used in the reference implementation
*********************************************************

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
    - nrf_wifi_osal_rand8_get()
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
This layer is pulled in as the following submodules:

* *Driver* : From the Zephyr Project nrf_wifi repository. For more information, visit the `Zephyr project nrf_wifi repository <https://github.com/zephyrproject-rtos/nrf_wifi>`_.
* *Firmware patches*: From the nRF Connect SDK nrfxlib repository. For more information, visit the `nRF Connect SDK nrfxlib repository <https://github.com/nrfconnect/sdk-nrfxlib>`_.

.. note ::

   The BM driver used the firmware patches from <https://github.com/nrfconnect/sdk-nrfxlib/tree/main/nrf_wifi/bin/zephyr>.

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

- `nrf70_bm_tx_pwr_ceil_dk.h`: Contains the TX power configuration for the nRF7002 Development Kit (DK).

- `nrf70_bm_tx_pwr_ceil_ek.h`: Contains the TX power configuration for the nRF7002 Evaluation Kit (EK).

For a new nRF70-based PCB design, the developer may create a new header file with the same structure as the reference files and replace the values with the appropriate values for the designed PCB.
The appropriate TX power ceiling values may be derived by running the radio test sample on the board and using a spectrum/vector signal analyzer to measure the output power, spectral mask, EVM and other parameters and then adjusting the TX power ceiling values to meet the requirements.
The values are represented in 0.25 dB increments.

For more information on TX power calculation, refer to the section `nRF70 TX power calculation <https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/drivers/wifi/nrf70_native.html#tx_power_calculation>`_.


Reference header file structure
-------------------------------

.. doxygenfile:: nrf70_bm_tx_pwr_ceil_dk.h


nRF70 OS agnostic layers API documentation
******************************************

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

Zephyr build compiler options and linker flags
**********************************************

The nRF70 Series BM driver uses the following compiler options and linker flags that are specific to the Zephyr build system.
The compiler used is ``Zephyr SDK`` which uses ``arm-zephyr-eabi-gcc``. For more information, visit the `Zephyr SDK documentation <https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html>`_.

Compiler options
----------------

Below is an example of the compiler options used by the nRF70 Series BM driver from `compile_commands.json` in the ``build`` directory:

.. code-block:: json
  :linenos:

  {
    "directory": "/home/nrf/work/bm_scan/nrf70-bm.git/build",
    "command": "/home/nrf/ncs/toolchains/e9dba88316/opt/zephyr-sdk/arm-zephyr-eabi/bin/arm-zephyr-eabi-gcc \
    -DCONFIG_NRF_WIFI_FW_BIN=/home/nrf/work/bm_scan/nrf70-bm.git/../nrfxlib/nrf_wifi/fw_bins/scan_only/nrf70.bin \
    -DKERNEL -DK_HEAP_MEM_POOL_SIZE=30000 -DNRF5340_XXAA_APPLICATION -DNRF_SKIP_FICR_NS_COPY_TO_RAM \
    -DPICOLIBC_LONG_LONG_PRINTF_SCANF -D__LINUX_ERRNO_EXTENSIONS__ -D__PROGRAM_START -D__ZEPHYR__=1 \
    -I/home/nrf/work/bm_scan/nrf70-bm.git/nrf70_bm_lib/include -I/home/nrf/work/bm_scan/zephyr/include/zephyr \
    -I/home/nrf/work/bm_scan/nrfxlib/nrf_wifi/utils/inc -I/home/nrf/work/bm_scan/nrfxlib/nrf_wifi/os_if/inc \
    -I/home/nrf/work/bm_scan/nrfxlib/nrf_wifi/bus_if/bus/qspi/inc -I/home/nrf/work/bm_scan/nrfxlib/nrf_wifi/bus_if/bal/inc \
    -I/home/nrf/work/bm_scan/nrfxlib/nrf_wifi/fw_if/umac_if/inc -I/home/nrf/work/bm_scan/nrfxlib/nrf_wifi/fw_load/mips/fw/inc \
    -I/home/nrf/work/bm_scan/nrfxlib/nrf_wifi/hw_if/hal/inc -I/home/nrf/work/bm_scan/nrfxlib/nrf_wifi/hw_if/hal/inc/fw \
    -I/home/nrf/work/bm_scan/nrfxlib/nrf_wifi/fw_if/umac_if/inc/fw -I/home/nrf/work/bm_scan/nrfxlib/nrf_wifi/fw_if/umac_if/inc/default \
    -I/home/nrf/work/bm_scan/zephyr/include -I/home/nrf/work/bm_scan/nrf70-bm.git/build/zephyr/include/generated \
    -I/home/nrf/work/bm_scan/zephyr/soc/nordic -I/home/nrf/work/bm_scan/zephyr/soc/nordic/nrf53/. \
    -I/home/nrf/work/bm_scan/zephyr/soc/nordic/common/. -I/home/nrf/work/bm_scan/nrf/include -I/home/nrf/work/bm_scan/nrf/tests/include \
    -I/home/nrf/work/bm_scan/modules/hal/cmsis/CMSIS/Core/Include -I/home/nrf/work/bm_scan/zephyr/modules/cmsis/. \
    -I/home/nrf/work/bm_scan/modules/hal/nordic/nrfx -I/home/nrf/work/bm_scan/modules/hal/nordic/nrfx/drivers/include \
    -I/home/nrf/work/bm_scan/modules/hal/nordic/nrfx/mdk -I/home/nrf/work/bm_scan/zephyr/modules/hal_nordic/nrfx/. \
    -isystem /home/nrf/work/bm_scan/zephyr/lib/libc/common/include -isystem /home/nrf/work/bm_scan/nrfxlib/crypto/nrf_cc312_platform/include \
    -fno-strict-aliasing -Os -imacros /home/nrf/work/bm_scan/nrf70-bm.git/build/zephyr/include/generated/autoconf.h \
    -fno-printf-return-value -fno-common -g -gdwarf-4 -fdiagnostics-color=always -mcpu=cortex-m33 -mthumb -mabi=aapcs -mfp16-format=ieee \
    -mtp=soft --sysroot=/home/nrf/ncs/toolchains/e9dba88316/opt/zephyr-sdk/arm-zephyr-eabi/arm-zephyr-eabi \
    -imacros /home/nrf/work/bm_scan/zephyr/include/zephyr/toolchain/zephyr_stdint.h -Wall -Wformat -Wformat-security -Wno-format-zero-length \
    -Wdouble-promotion -Wno-pointer-sign -Wpointer-arith -Wexpansion-to-defined -Wno-unused-but-set-variable -Werror=implicit-int -fno-pic \
    -fno-pie -fno-asynchronous-unwind-tables -ftls-model=local-exec -fno-reorder-functions --param=min-pagesize=0 -fno-defer-pop \
    -fmacro-prefix-map=/home/nrf/work/bm_scan/nrf70-bm.git/samples/scan_bm=CMAKE_SOURCE_DIR -fmacro-prefix-map=/home/nrf/work/bm_scan/zephyr=ZEPHYR_BASE \
    -fmacro-prefix-map=/home/nrf/work/bm_scan=WEST_TOPDIR -ffunction-sections -fdata-sections --specs=picolibc.specs -std=c99 \
    -o modules/nrf70-bm.git/nrf70_bm_lib/CMakeFiles/nrf70-bm-lib.dir/source/nrf70_bm_lib.c.obj -c /home/nrf/work/bm_scan/nrf70-bm.git/nrf70_bm_lib/source/nrf70_bm_lib.c",
    "file": "/home/nrf/work/bm_scan/nrf70-bm.git/nrf70_bm_lib/source/nrf70_bm_lib.c",
    "output": "modules/nrf70-bm.git/nrf70_bm_lib/CMakeFiles/nrf70-bm-lib.dir/source/nrf70_bm_lib.c.obj"
  }

Linker flags
------------

Below is an example of the linker flags used by the nRF70 Series BM driver from `build.ninja` and `CMakeFiles/rules.ninja` in the ``build`` directory:

.. code-block:: none

  LINK_FLAGS = -gdwarf-4
  # Rule for linking C static library.

  rule C_STATIC_LIBRARY_LINKER__nrf70-bm-lib_
    command = $PRE_LINK && /usr/bin/cmake -E rm -f $TARGET_FILE && /home/nrf/ncs/toolchains/e9dba88316/opt/zephyr-sdk/arm-zephyr-eabi/bin/arm-zephyr-eabi-ar qc $TARGET_FILE $LINK_FLAGS $in && /home/nrf/ncs/toolchains/e9dba88316/opt/zephyr-sdk/arm-zephyr-eabi/bin/arm-zephyr-eabi-ranlib $TARGET_FILE && $POST_BUILD
    description = Linking C static library $TARGET_FILE
    restat = $RESTAT
