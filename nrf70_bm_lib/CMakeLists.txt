#
# Copyright (c) 2024 Nordic Semiconductor
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Check if ZEPHYR_BASE is set
if(NOT DEFINED ENV{ZEPHYR_BASE})
    message(FATAL_ERROR "ZEPHYR_BASE environment variable is not set. Please set it to the Zephyr base directory.")
endif()

if (CONFIG_NRF70_BM_LIB)
  add_library(nrf70-bm-lib STATIC)

  set(NRF_WIFI_DIR ${ZEPHYR_CURRENT_MODULE_DIR}/../nrfxlib/nrf_wifi)

  target_include_directories(nrf70-bm-lib PUBLIC
    include
    # OS agnostic library
    $<TARGET_PROPERTY:nrf-wifi,PUBLIC_INCLUDE_DIRECTORIES>
    # Inlucde till the last directory to avoid using Zephyr in BM code
    ${ZEPHYR_BASE}/include/zephyr
  )

  target_compile_definitions_ifndef(CONFIG_NRF70_RADIO_TEST
    nrf70-bm-lib
    PRIVATE
    -DCONFIG_NRF_WIFI_FW_BIN=${NRF_WIFI_DIR}/fw_bins/scan_only/nrf70.bin
  )

  target_compile_definitions_ifdef(CONFIG_NRF70_RADIO_TEST
    nrf70-bm-lib
    PRIVATE
    -DCONFIG_NRF_WIFI_FW_BIN=${NRF_WIFI_DIR}/fw_bins/radio_test/nrf70.bin
  )

  target_compile_definitions_ifdef(CONFIG_NRF70_RADIO_TEST
    nrf70-bm-lib
    PRIVATE
    -DCONFIG_NRF700X_RADIO_TEST
  )


  target_sources(
    nrf70-bm-lib
    PRIVATE
    source/nrf70_bm_lib.c
    source/nrf70_bm_core.c
  )

  target_link_libraries(nrf70-bm-lib PRIVATE nrf-wifi nrf70-zep-shim)
endif()
