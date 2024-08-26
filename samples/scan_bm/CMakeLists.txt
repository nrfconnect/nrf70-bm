#
# Copyright (c) 2024 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nrf70_scan_bm)

target_include_directories(app PRIVATE
  $<TARGET_PROPERTY:nrf70-bm-lib,INTERFACE_INCLUDE_DIRECTORIES>
)

target_sources(
  app
  PRIVATE
  src/main.c
  src/utils.c
)

target_link_libraries(app PRIVATE nrf70-bm-lib)
