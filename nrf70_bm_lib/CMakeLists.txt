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
  set(NRF_WIFI_DIR ${ZEPHYR_NRF_WIFI_MODULE_DIR})
  set(NRF_WIFI_BINS_DIR ${ZEPHYR_NRFXLIB_MODULE_DIR}/nrf_wifi/bin/zephyr)

  add_subdirectory(${NRF_WIFI_DIR} nrf-wifi-osal)

  add_library(nrf70-bm-lib STATIC)


  target_include_directories(nrf70-bm-lib PUBLIC
    include
    # Include till the last directory to avoid using Zephyr in BM code
    ${ZEPHYR_BASE}/include/zephyr
  )

  target_compile_definitions_ifdef(CONFIG_NRF70_BM_SCAN_ONLY
    nrf70-bm-lib
    PRIVATE
    -DCONFIG_NRF_WIFI_SYS_FW_BIN=${NRF_WIFI_BINS_DIR}/scan_only/nrf70.bin
  )

  target_compile_definitions_ifdef(CONFIG_NRF70_BM_RADIO_TEST
    nrf70-bm-lib
    PRIVATE
    -DCONFIG_NRF_WIFI_RT_FW_BIN=${NRF_WIFI_BINS_DIR}/radio_test/nrf70.bin
  )

  target_compile_definitions_ifdef(CONFIG_NRF70_BM_RADIO_TEST
    nrf70-bm-lib
    PRIVATE
    -DCONFIG_NRF70_BM_RADIO_TEST
  )

  target_compile_definitions_ifdef(CONFIG_NRF70_BM_SCAN_ONLY
    nrf70-bm-lib
    PRIVATE
    -DCONFIG_NRF70_BM_SCAN_ONLY
  )


  target_sources(
    nrf70-bm-lib
    PRIVATE
    source/common/nrf70_bm_core.c
  )

if(CONFIG_NRF70_BM_RADIO_TEST)
  target_sources(
    nrf70-bm-lib
    PRIVATE
    source/radio_test/nrf70_bm_lib.c
    source/radio_test/nrf70_bm_core.c
  )
endif()

if(CONFIG_NRF70_BM_SCAN_ONLY)
  target_sources(
    nrf70-bm-lib
    PRIVATE
    source/system/nrf70_bm_lib.c
    source/system/nrf70_bm_core.c
  )
endif()

  target_link_libraries(nrf70-bm-lib PRIVATE nrf70-zep-shim nrf-wifi-osal)
endif()
