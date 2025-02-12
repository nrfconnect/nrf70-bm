/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief nRF70 Bare Metal library.
 * @defgroup nrf70_bm nRF70 Bare Metal library
 */

#ifndef NRF70_BM_COMMON_LIB_H__
#define NRF70_BM_COMMON_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/** @brief Log levels */
#if CONFIG_NRF70_BM_LOG_LEVEL > 0
#define NRF70_LOG_ERR(...) printf(__VA_ARGS__); printf("\n")
#else
#define NRF70_LOG_ERR(...)
#endif

#if CONFIG_NRF70_BM_LOG_LEVEL > 1
#define NRF70_LOG_WRN(...) printf(__VA_ARGS__); printf("\n")
#else
#define NRF70_LOG_WRN(...)
#endif

#if CONFIG_NRF70_BM_LOG_LEVEL > 2
#define NRF70_LOG_INF(...) printf(__VA_ARGS__); printf("\n")
#else
#define NRF70_LOG_INF(...)
#endif

#if CONFIG_NRF70_BM_LOG_LEVEL > 3
#define NRF70_LOG_DBG(...) printf(__VA_ARGS__); printf("\n")
#else
#define NRF70_LOG_DBG(...)
#endif

/** @brief Maximum number of channels supported by the Wi-Fi chip - 2.4GHz + 5GHz. */
#define NRF70_MAX_CHANNELS ( 14 + 30 )

/** @brief Per-channel regulatory attributes */
struct nrf70_bm_reg_chan_info {
	/** Center frequency in MHz */
	unsigned short center_frequency;
	/** Maximum transmission power (in dBm) */
	unsigned short max_power:8;
	/** Is channel supported or not */
	unsigned short supported:1;
	/** Passive transmissions only */
	unsigned short passive_only:1;
	/** Is a DFS channel */
	unsigned short dfs:1;
};



/** @brief Regulatory information */
struct nrf70_bm_regulatory_info {
	/** Country code - ISO/IEC 3166-1 alpha-2 */
	char country_code[2];
	/** Force - ignore 802.11d beacon hints (only used in SET) */
	bool force;
	/** Number of channels supported (only used in GET) */
	unsigned int num_channels;
	/** Channels information (only used in GET) */
	struct nrf70_bm_reg_chan_info *chan_info;
};

#ifdef __cplusplus
}
#endif

#endif /* NRF70_BM_COMMON_LIB_H__ */
