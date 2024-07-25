#
# Copyright (c) 2024 Nordic Semiconductor
#
# SPDX-License-Identifier: BSD-3-Clause
#

if (CONFIG_NRF70_BM_LIB)
  add_library(nrf70-bm-lib STATIC)

  set(NRF_WIFI_DIR ${ZEPHYR_CURRENT_MODULE_DIR}/../nrfxlib/nrf_wifi)

  target_include_directories(nrf70-bm-lib PUBLIC
    include
    # OS agnostic library
    $<TARGET_PROPERTY:nrf-wifi,PUBLIC_INCLUDE_DIRECTORIES>
  )

  target_compile_definitions(nrf70-bm-lib PRIVATE
    -DCONFIG_NRF_WIFI_FW_BIN=${NRF_WIFI_DIR}/fw_bins/scan_only/nrf70.bin
  )


  target_sources(
    nrf70-bm-lib
    PRIVATE
    source/nrf70_bm_lib.c
    source/nrf70_bm_init.c
  )

  target_link_libraries(nrf70-bm-lib PRIVATE nrf-wifi nrf70-zep-shim)
endif()
