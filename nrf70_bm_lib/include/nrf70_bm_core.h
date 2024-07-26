/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief nRF70 Bare Metal library initialization.
 */
#ifndef NRF70_BM_INIT_H__
#define NRF70_BM_INIT_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>


#include <fmac_api.h>

extern struct nrf70_wifi_drv_priv_bm nrf70_bm_priv;

#define NRF70_BM_NUM_VIFS 1

struct nrf70_wifi_vif_bm {
	unsigned char vif_idx;
	bool scan_in_progress;
	unsigned short max_bss_cnt;
	unsigned short scan_res_cnt;
	unsigned char mac_addr[6];
	enum nrf_wifi_fmac_if_op_state op_state;
	bool scan_done;
	void (*scan_result_cb)(void *result);
};

struct nrf70_wifi_ctx_bm {
	void *rpu_ctx;
	struct nrf70_wifi_vif_bm vifs[NRF70_BM_NUM_VIFS];
};

struct nrf70_wifi_drv_priv_bm {
	struct nrf_wifi_fmac_priv *fmac_priv;
	struct nrf70_wifi_ctx_bm rpu_ctx_bm;
};

int nrf70_fmac_init(void);
int nrf70_fmac_deinit(void);
int nrf70_fmac_add_vif_sta(void);
int nrf70_fmac_del_vif_sta(void);

#endif /* NRF70_BM_INIT_H__ */