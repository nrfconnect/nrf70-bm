#
# Copyright (c) 2024 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nrf70_scan_rt_bm)

set(NRF70_BM_DIR ../../nrf70_bm_lib)
set(NRF_WIFI_DIR ${ZEPHYR_NRF_WIFI_MODULE_DIR})

target_compile_definitions(app PRIVATE
	-DCONFIG_NRF70_BM_RADIO_TEST
	-DNRF70_RADIO_TEST # OSAL
  -DCONFIG_NRF70_BM_SCAN_ONLY
  -DNRF70_SCAN_ONLY # OSAL
)

target_include_directories(app PRIVATE
  src
  ${NRF70_BM_DIR}/include
	${NRF_WIFI_DIR}/utils/inc
	${NRF_WIFI_DIR}/os_if/inc
	${NRF_WIFI_DIR}/bus_if/bus/qspi/inc
	${NRF_WIFI_DIR}/bus_if/bal/inc
	${NRF_WIFI_DIR}/fw_load/mips/fw/inc
	${NRF_WIFI_DIR}/hw_if/hal/inc
	${NRF_WIFI_DIR}/hw_if/hal/inc/fw
	${NRF_WIFI_DIR}/fw_if/umac_if/inc/fw
	${NRF_WIFI_DIR}/fw_if/umac_if/inc/
)

target_sources(
  app
  PRIVATE
  src/main.c
  src/utils.c
)

if(DEFINED CONFIG_NRF70_DEBUG_SHELL)
  target_include_directories(app PRIVATE
    $<TARGET_PROPERTY:nrf-wifi-osal,INTERFACE_INCLUDE_DIRECTORIES>
  )

endif()

target_link_libraries(app PRIVATE nrf70-bm-lib)
