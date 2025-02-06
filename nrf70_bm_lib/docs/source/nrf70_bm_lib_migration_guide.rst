:orphans:

Migration guide
###############

The migration guide provides information on the changes introduced in the nRF70 BM library.
The guide is intended for users who are migrating from an older version of the library to a newer version.

Versioning scheme
=================

The nRF70 BM library follows the `NCS versioning scheme <https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/dev_model_and_contributions/code_base.html#versions_and_revisions>`_.

.. _migration_1.1.0:

Migration guide to nRF70 BM library v1.1.0
*******************************************

The following changes were introduced in the nRF70 BM library v1.1.0:

API changes
===========

* The following APIs were added to the library:

  * ``nrf70_bm_sys_get_reg()``
  * ``nrf70_bm_rt_init()``
  * ``nrf70_bm_rt_deinit()``

* The following APIs were modified in the library:

  * ``nrf70_bm_init()`` is now renamed to ``nrf70_bm_sys_init()``.
      - added a new parameter ``mac_addr`` to allow the user to set the MAC address of the device.
      - added a new parameter ``reg_info`` to allow the user to set the regulatory information of the device.
  * ``nrf70_bm_deinit()`` is now renamed to ``nrf70_bm_sys_deinit()``.
  * ``nrf70_bm_scan_start()`` is now renamed to ``nrf70_bm_sys_scan_start()``.
  * ``nrf70_bm_set_reg()`` is now renamed to ``nrf70_bm_sys_set_reg()``.
  * ``nrf70_bm_mac_txt()`` is now renamed to ``nrf70_bm_sys_mac_txt``.
  * ``nrf70_bm_dump_stats()`` is now renamed to ``nrf70_bm_sys_dump_stats()``.

Configuration changes
=====================

* The following configuration options were added to the library:

  * ``CONFIG_NRF70_REG_DOMAIN``

Build system changes
====================

* For the build system changes refer to the following links:

  * <https://github.com/zephyrproject-rtos/nrf_wifi/commit/cc1008dea1d8171f3996c4185acabdc0f9f18e62>
  * <>