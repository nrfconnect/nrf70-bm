/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief nRF70 Bare Metal library initialization.
 */
#ifndef NRF70_BM_RT_CORE_H__
#define NRF70_BM_RT_CORE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

#include "common/nrf70_bm_core.h"
#include <radio_test/fmac_api.h>

struct nrf70_bm_rt_wifi_ctx {
	void *rpu_ctx;
	struct rpu_conf_params conf_params;
	bool rf_test_run;
	unsigned char rf_test;
};

struct nrf70_bm_rt_wifi_drv_priv {
	struct nrf_wifi_fmac_priv *fmac_priv;
	struct nrf70_bm_rt_wifi_ctx rpu_ctx_bm;
};


int nrf70_bm_rt_fmac_init(void);
int nrf70_bm_rt_fmac_deinit(void);
enum nrf_wifi_status nrf70_bm_rt_fw_load(void *rpu_ctx);

#endif /* NRF70_BM_RT_CORE_H__ */
