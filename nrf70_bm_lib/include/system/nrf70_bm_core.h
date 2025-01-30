/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief nRF70 Bare Metal library initialization.
 */
#ifndef NRF70_BM_SYS_CORE_H__
#define NRF70_BM_SYS_CORE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

#include <system/fmac_api.h>
#include "common/nrf70_bm_core.h"

#define NRF70_BM_NUM_VIFS 1

struct nrf70_bm_sys_wifi_vif {
	unsigned char vif_idx;
	bool scan_in_progress;
	unsigned short max_bss_cnt;
	unsigned short scan_res_cnt;
	unsigned char mac_addr[6];
	enum nrf_wifi_fmac_if_op_state op_state;
	bool scan_done;
	void (*scan_result_cb)(void *result);
};

struct nrf70_bm_sys_wifi_ctx {
	void *rpu_ctx;
	struct nrf70_bm_sys_wifi_vif vifs[NRF70_BM_NUM_VIFS];
};

struct nrf70_bm_sys_wifi_drv_priv {
	struct nrf_wifi_fmac_priv *fmac_priv;
	struct nrf70_bm_sys_wifi_ctx rpu_ctx_bm;
};


int nrf70_bm_sys_fmac_init(void);
int nrf70_bm_sys_fmac_deinit(void);
int nrf70_bm_sys_fmac_get_reg(struct nrf70_bm_regulatory_info *reg_info);
int nrf70_bm_sys_fmac_set_reg(struct nrf70_bm_regulatory_info *reg_info);
int nrf70_bm_sys_fmac_add_vif_sta(uint8_t *mac_addr);
int nrf70_bm_sys_fmac_del_vif_sta(void);
enum nrf_wifi_status nrf70_bm_sys_fw_load(void *rpu_ctx);

#endif /* NRF70_BM_SYS_CORE_H__ */
