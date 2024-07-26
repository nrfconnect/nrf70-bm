#
# Copyright (c) 2024 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: BSD-3-Clause
#

if (CONFIG_NRF70_ZEPHYR_SHIM)
  set(NRF_WIFI_DIR ${ZEPHYR_CURRENT_MODULE_DIR}/../nrfxlib/nrf_wifi)

  zephyr_library_named(nrf70-zep-shim)

  zephyr_library_include_directories(
    include
    ${NRF_WIFI_DIR}/os_if/inc
  )

  zephyr_library_sources(source/shim.c)
  zephyr_library_sources(source/qspi_if.c)
  zephyr_library_sources(source/rpu_hw_if.c)
  zephyr_library_sources(source/device.c)
  zephyr_library_sources(source/work.c)
  zephyr_library_sources(source/timer.c)
endif()
