# Copyright (c) 2024 Nordic Semiconductor ASA
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory_ifdef(CONFIG_NRF70_BM_LIB nrf70_bm_lib)
add_subdirectory_ifdef(CONFIG_NRF70_ZEPHYR_SHIM nrf70_zephyr_shim)
