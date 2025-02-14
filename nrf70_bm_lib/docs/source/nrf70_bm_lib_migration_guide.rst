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
  * ``nrf70_bm_rt_set_reg()``
  * ``nrf70_bm_rt_get_reg()``

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

nRF70 BM library
----------------

The following build system changes were introduced in the nRF70 BM library v1.1.0:

.. list-table:: Build system modifications
  :header-rows: 1

  * - Old Path
    - New Path(s)
  * - ``nrf70_bm_lib/source/nrf70_bm_lib.c``
    - | ``nrf70_bm_lib/source/nrf70_bm_lib/system/nrf70_bm_lib.c``
      | ``nrf70_bm_lib/source/nrf70_bm_lib/radio_test/nrf70_bm_lib.c``
  * - ``nrf70_bm_lib/source/nrf70_bm_core.c``
    - | ``nrf70_bm_lib/source/nrf70_bm_lib/common/nrf70_bm_core.c``
      | ``nrf70_bm_lib/source/nrf70_bm_lib/system/nrf70_bm_core.c``
      | ``nrf70_bm_lib/source/nrf70_bm_lib/radio_test/nrf70_bm_core.c``
  * - ``nrf70_bm_lib/include/nrf70_bm_lib.h``
    - | ``nrf70_bm_lib/include/nrf70_bm_lib/common/nrf70_bm_lib.h``
      | ``nrf70_bm_lib/include/nrf70_bm_lib/system/nrf70_bm_lib.h``
      | ``nrf70_bm_lib/include/nrf70_bm_lib/radio_test/nrf70_bm_lib.h``
  * - ``nrf70_bm_lib/include/nrf70_bm_core.h``
    - | ``nrf70_bm_lib/include/common/nrf70_bm_core.h``
      | ``nrf70_bm_lib/include/system/nrf70_bm_core.h``
      | ``nrf70_bm_lib/include/radio_test/nrf70_bm_core.h``
  * - ``nrf70_bm_lib/include/nrf70_bm_tx_pwr_ceil_dk.h``
    - ``nrf70_bm_lib/include/common/nrf70_bm_tx_pwr_ceil_dk.h``
  * - ``nrf70_bm_lib/include/nrf70_bm_tx_pwr_ceil_ek.h``
    - ``nrf70_bm_lib/include/common/nrf70_bm_tx_pwr_ceil_ek.h``
