.. _nrf70_bm_lib:

Introduction
############

nRF70 Bare Metal (BM) library is a library that provides a set of APIs to interact with the nRF70 Series of ICs.
The library is not dependent on any RTOS. As such it may be used in a bare metal environment, as well as a
third-party RTOS environment (not Zephyr RTOS or nRF Connect SDK). This allows developers to easily port the
library to any platform of their choice.

Architecture
############

The software architecture of the nRF70 BM library is presented in the following figure.

.. figure:: ./images/nrf70_bm_architecture.png
   :alt: nRF70 BM stack architecture
   :align: center
   :figclass: align-center

   nRF70 Bare Metal stack architecture overview


OS-agnostic library
*******************

The library exposes the following functionality to the user application

* nRF70 Series device initialization and de-initialization
* Wi-Fi scan, through a single-function API supporting a wide list of scan configuration parameters
* Obtaining statistics from the nRF70 Series device

The library is described in a single public-API definition header file ``nrf70_bm_lib.h``.

Being fully RTOS-agnostic, the BM library is portable to any bare-metal or OS environment.

The user API of the library is fully described in :ref:`nrf70_bm_api`.

OS-agnostic driver layer
************************

The BM library uses the OS-agnostic nRF70 Wi-Fi driver layer to interact with the nRF70 Series device.
Only a subset of the OS-agnostic (FMAC) layer is required to support the scan and radio-test functionality
of the nRF70 BM library.

nRF70 Shim layer
****************

The nRF70 shim layer contains a reference implementation of the bare metal library for the Zephyr RTOS
and nRF Connect SDK. The reference implementation serves two main purposes

* It allows users to easily build, test, and evaluate the nRF70 BM library on Nordic evaluation boards
* It can be used as a guide for porting the library to other platforms and OS environments.

Shim layer structure
====================

This reference implementation is structured in multiple source directories as follows:

.. code-block:: none

    the nRF70 Series_zephyr_shim/     # Zephyr shim for the nRF70 Series, reference for third-party host platforms
      include/             # Include directory
      source/              # Source directory
        bus/               # Bus interface
        os/                # OS shim
        platform/          # Platform specific files
      CMakeLists.txt       # CMake build file```

The key components of the Zephyr shim reference implementation are:

* ``os``: Contains the OS shim implementation for Zephyr.
* ``bus``: Contains the bus (data transfer) interface implementation for SPI and QSPI.
* ``platform``: Contains the platform specific files, and has an RPU (Radio Processing Unit) abstraction layer that interacts with the nRF70 Series device,
  either through QSPI or SPI. The platform also uses Zephyr GPIO APIs to manage GPIO pins of the nRF70 Series.


Design essentials
#################

nRF70 BM library threading model
********************************

The nRF70 Series BM library and driver use a simple threading model to interact with the nRF70 Series device.
The library driver code execute in the following contexts:"

* *Application (thread) context*: Regular application thread context for invoking the nRF70 BM library APIs to to interact with the nRF70 Series device.
  All API functions execute fully in thread context (i.e. there is no tasklet offload) running to completion.

* *Interrupt context*: For handling interrupts from the nRF70 Series. Interrupt service routines are used to schedule tasklets to offload the nRF70 Series event handling.
  The nRF70 device requires a single  `host IRQ` interrupt line to raise interrupts on the host platform, when the device needs to report an event. A GPIO pin needs to be configured as a `host IRQ` at the host MCU.
  The interrupt service routine reads the event coming from the nRF70 Series device and schedules a tasklet to handle the event.

* *Tasklet context*: Tasklets perform the actual work of interacting with the nRF70, processing events coming from the device (offloaded tasks from ISRs)
  Only event receive operations are performned in tasklets. Essentially, event receive tasklets read the event data coming from the nRF70 Series device and hand them over to the registered FMAC callbacks.
  An example of such operation is the processing of incoming AP scan results after a scan command has been issued. 

  .. note::
     In the reference implementation for Zephyr tasklet work is offloaded to Zephyr kernel workqueues.

