/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief nRF70 Bare Metal library initialization.
 */
#ifndef NRF70_BM_COMMON_CORE_H__
#define NRF70_BM_COMMON_CORE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

#include "host_rpu_sys_if.h"
#include "common/nrf70_bm_lib.h"

/* INCBIN macro Taken from https://gist.github.com/mmozeiko/ed9655cf50341553d282 */
#define STR2(x) #x
#define STR(x) STR2(x)

#ifdef __APPLE__
#define USTR(x) "_" STR(x)
#else
#define USTR(x) STR(x)
#endif

#ifdef _WIN32
#define INCBIN_SECTION ".rdata, \"dr\""
#elif defined __APPLE__
#define INCBIN_SECTION "__TEXT,__const"
#else
#define INCBIN_SECTION ".rodata.*"
#endif

/* This aligns start address to 16 and terminates byte array with explicit 0
 * which is not really needed, feel free to change it to whatever you want/need
 */
#define INCBIN(prefix, name, file) \
	__asm__(".section " INCBIN_SECTION "\n" \
			".global " USTR(prefix) "_" STR(name) "_start\n" \
			".balign 16\n" \
			USTR(prefix) "_" STR(name) "_start:\n" \
			".incbin \"" file "\"\n" \
			\
			".global " STR(prefix) "_" STR(name) "_end\n" \
			".balign 1\n" \
			USTR(prefix) "_" STR(name) "_end:\n" \
			".byte 0\n" \
	); \
	extern __aligned(16)    const char prefix ## _ ## name ## _start[]; \
	extern                  const char prefix ## _ ## name ## _end[];

void nrf70_bm_conf_board_dep_params(struct nrf_wifi_board_params *board_params);
void nrf70_bm_conf_tx_pwr_settings(struct nrf_wifi_tx_pwr_ctrl_params *tx_pwr_ctrl_params,
				   struct nrf_wifi_tx_pwr_ceil_params *tx_pwr_ceil_params);
enum nrf_wifi_status nrf70_bm_fw_load(void *rpu_ctx,
				      const uint8_t *fw_start,
				      const uint8_t *fw_end);
#endif /* NRF70_BM_COMMON_CORE_H__ */
