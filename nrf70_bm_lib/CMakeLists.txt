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
  if (DEFINED ENV{ZEPHYR_BASE})
    # Use Zephyr module
    set(NRF_WIFI_DIR ${ZEPHYR_NRF_WIFI_MODULE_DIR})
    set(NRF_WIFI_BINS_DIR ${ZEPHYR_NRFXLIB_MODULE_DIR}/nrf_wifi/bin/zephyr)
  else()
    # Use git submodule
    set(NRF_WIFI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../sdk-nrfxlib/nrf_wifi)
    set(NRF_WIFI_BINS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../sdk-nrfxlib/nrf_wifi/bin/zephyr)
  endif()

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

  target_compile_options(nrf70-bm-lib PRIVATE
    -Wall                     # All warnings
    -Wextra                   # Extra warnings
    -Werror                   # Warnings as errors
    -Wformat=2                # Format string checks
    -Wformat-security         # Security issues with format strings
    -Wnull-dereference        # Null pointer dereferences
    -Wstack-protector         # Stack protection
    -fstack-protector-strong  # Strong stack protection
    -Warray-bounds            # Array bounds checking
    -Wstringop-overflow       # String operation overflow
    -Wconversion              # Implicit conversions
    -Wsign-conversion         # Sign conversions
    -Wmissing-include-dirs    # Missing include directories
    -Wcast-align              # Pointer cast alignment
    -Wcast-qual               # Pointer cast dropping qualifiers
    -Wdiscarded-qualifiers    # Discarded qualifiers
    -Wduplicated-branches     # Duplicated branches
    -Wduplicated-cond         # Duplicated conditions
    -Wlogical-op              # Suspicious logical operators
    -Wnull-dereference        # Null pointer dereferences
    -Wjump-misses-init        # Goto skipping variable initialization
    -Wunused-parameter        # Unused parameters
    -Wshift-overflow=2        # Shift overflows
    -Wmissing-prototypes      # Missing function prototypes
    -Wredundant-decls         # Redundant declarations
    -Wtype-limits             # Type limits
    -Wshadow                  # Shadowed variables
    -Wstrict-prototypes       # Strict prototypes
    -Wundef                   # Undefined macros
    -Wuninitialized           # Uninitialized variables
    -Wpointer-arith           # Pointer arithmetic
    -Wbad-function-cast       # Bad function casts
    -Wstrict-overflow         # Strict overflow
    -Winline                  # Inlining issues
    -Wnested-externs          # Nested externs
    -Wold-style-definition    # Old-style definitions
    -Wmissing-declarations    # Missing declarations
    -Wmissing-noreturn        # Missing noreturn
    -Wdisabled-optimization   # Disabled optimizations
    -Wfloat-equal             # Floating point equality
    -Winit-self               # Initialization to self
    -Wswitch-default          # Missing switch default
    -Wswitch-enum             # Missing switch enum
    -Wunsafe-loop-optimizations # Unsafe loop optimizations
    -Wlogical-not-parentheses # Logical not parentheses
    -Wstrict-aliasing         # Strict aliasing
    -Wvariadic-macros         # Variadic macros
    -Wvolatile-register-var   # Volatile register variables
  )
endif()
