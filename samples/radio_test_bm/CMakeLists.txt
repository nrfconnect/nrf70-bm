#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nrf_wifi_radio_test)

set(NRF_WIFI_DIR ../../../nrfxlib/nrf_wifi)
set(NRF70_BM_DIR ../../nrf70_bm_lib)
set(NRF70_SHIM_DIR ../../nrf70_zephyr_shim)

target_compile_definitions(app PRIVATE
	-DCONFIG_NRF700X_RADIO_TEST # OSAL
	-DCONFIG_NRF70_RADIO_TEST # BM
)

target_include_directories(app PRIVATE
	inc
	${NRF_WIFI_DIR}/utils/inc
	${NRF_WIFI_DIR}/os_if/inc
	${NRF_WIFI_DIR}/bus_if/bus/qspi/inc
	${NRF_WIFI_DIR}/bus_if/bal/inc
	${NRF_WIFI_DIR}/fw_if/umac_if/inc
	${NRF_WIFI_DIR}/fw_load/mips/fw/inc
	${NRF_WIFI_DIR}/hw_if/hal/inc
	${NRF_WIFI_DIR}/hw_if/hal/inc/fw
	${NRF_WIFI_DIR}/fw_if/umac_if/inc/fw
	${NRF_WIFI_DIR}/fw_if/umac_if/inc/radio_test
	${NRF70_BM_DIR}/include
	${NRF70_SHIM_DIR}/include
)

target_sources(app PRIVATE
	src/nrf_wifi_radio_test_main.c
	src/nrf_wifi_radio_test_shell.c
	src/nrf_wifi_radio_ficr_shell.c
)

target_link_libraries(app PRIVATE nrf-wifi nrf70-zep-shim nrf70-bm-lib)