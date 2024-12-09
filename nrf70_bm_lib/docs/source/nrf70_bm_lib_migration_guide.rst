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

  * ``nrf70_bm_get_reg()``

* The following APIs were modified in the library:

  * ``nrf70_bm_init()``
      - added a new parameter ``mac_addr`` to allow the user to set the MAC address of the device.
      - added a new parameter ``reg_info`` to allow the user to set the regulatory information of the device.

Configuration changes
=====================

* The following configuration options were added to the library:

  * ``CONFIG_NRF70_REG_DOMAIN``
