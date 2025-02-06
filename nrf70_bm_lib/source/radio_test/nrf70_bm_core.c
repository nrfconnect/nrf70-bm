/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief nRF70 Bare Metal initialization.
 */
#include "radio_test/nrf70_bm_core.h"
#include "util.h"
#include "radio_test/fmac_api.h"
#include "common/fmac_util.h"

struct nrf70_bm_rt_wifi_drv_priv nrf70_bm_rt_priv;
extern const struct nrf_wifi_osal_ops nrf_wifi_os_bm_ops;
INCBIN(_bin, nrf70_bm_rt_fw, STR(CONFIG_NRF_WIFI_RT_FW_BIN));

int nrf70_bm_rt_fmac_init(void)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	unsigned int fw_ver = 0;
	void *rpu_ctx = nrf70_bm_rt_priv.rpu_ctx_bm.rpu_ctx;
	struct nrf_wifi_tx_pwr_ctrl_params tx_pwr_ctrl_params = { 0 };
	/* TODO: Hardcoded to 10 dBm, take as parameter */
	struct nrf_wifi_tx_pwr_ceil_params tx_pwr_ceil_params;
	struct nrf_wifi_board_params board_params = { 0 };
	bool enable_bf = false;
	enum op_band op_band = CONFIG_NRF_WIFI_OP_BAND;
#ifdef CONFIG_NRF_WIFI_LOW_POWER
	int sleep_type = SLEEP_DISABLE;
#endif /* CONFIG_NRF_WIFI_LOW_POWER */

	NRF70_LOG_DBG("Initializing FMAC module in radio test mode");

	nrf_wifi_osal_init(&nrf_wifi_os_bm_ops);

	nrf70_bm_rt_priv.fmac_priv = nrf_wifi_rt_fmac_init();

	if (!nrf70_bm_rt_priv.fmac_priv) {
		NRF70_LOG_ERR("Failed to initialize FMAC module\n");
		goto err;
	}

	rpu_ctx = nrf_wifi_rt_fmac_dev_add(nrf70_bm_rt_priv.fmac_priv,
					   &nrf70_bm_rt_priv.rpu_ctx_bm);

	if (!rpu_ctx) {
		NRF70_LOG_ERR("Failed to add device\n");
		goto deinit;
	}

	nrf70_bm_rt_priv.rpu_ctx_bm.rpu_ctx = rpu_ctx;

	status = nrf70_bm_rt_fw_load(rpu_ctx);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("Failed to load firmware\n");
		goto deinit;
	}

	status = nrf_wifi_fmac_ver_get(rpu_ctx, &fw_ver);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("Failed to get FMAC version\n");
		goto deinit;
	}

	NRF70_LOG_INF("Firmware (v%d.%d.%d.%d) booted successfully",
		NRF_WIFI_UMAC_VER(fw_ver),
		NRF_WIFI_UMAC_VER_MAJ(fw_ver),
		NRF_WIFI_UMAC_VER_MIN(fw_ver),
		NRF_WIFI_UMAC_VER_EXTRA(fw_ver));

	nrf70_bm_conf_tx_pwr_settings(&tx_pwr_ctrl_params,
				  &tx_pwr_ceil_params);

	nrf70_bm_conf_board_dep_params(&board_params);

#ifdef CONFIG_NRF_WIFI_BEAMFORMING
	enable_bf = true;
#endif

	status = nrf_wifi_rt_fmac_dev_init(rpu_ctx,
#ifdef CONFIG_NRF_WIFI_LOW_POWER
					   sleep_type,
#endif /* CONFIG_NRF_WIFI_LOW_POWER */
					   NRF_WIFI_DEF_PHY_CALIB,
					   op_band,
					   enable_bf,
					   &tx_pwr_ctrl_params,
					   &tx_pwr_ceil_params,
					   &board_params,
					   STRINGIFY(CONFIG_NRF70_REG_DOMAIN));
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("Failed to initialize device\n");
		goto deinit;
	}

	NRF70_LOG_DBG("FMAC module initialized");

	return 0;
deinit:
	nrf_wifi_osal_deinit();
	nrf70_bm_rt_fmac_deinit();
err:
	return -1;
}


int nrf70_bm_rt_fmac_deinit(void)
{
	NRF70_LOG_DBG("Deinitializing FMAC module");

	if (nrf70_bm_rt_priv.rpu_ctx_bm.rpu_ctx) {
		nrf_wifi_rt_fmac_dev_deinit(nrf70_bm_rt_priv.rpu_ctx_bm.rpu_ctx);
		nrf_wifi_fmac_dev_rem(nrf70_bm_rt_priv.rpu_ctx_bm.rpu_ctx);
	}

	if (nrf70_bm_rt_priv.fmac_priv) {
		nrf_wifi_fmac_deinit(nrf70_bm_rt_priv.fmac_priv);
	}

	nrf_wifi_osal_deinit();
	nrf70_bm_rt_priv.fmac_priv = NULL;
	nrf70_bm_rt_priv.rpu_ctx_bm.rpu_ctx = NULL;

	NRF70_LOG_DBG("FMAC module deinitialized");

	return 0;
}

enum nrf_wifi_status nrf70_bm_rt_fw_load(void *rpu_ctx)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	const uint8_t *fw_start = NULL;
	const uint8_t *fw_end = NULL;

	fw_start = _bin_nrf70_bm_rt_fw_start;
	fw_end = _bin_nrf70_bm_rt_fw_end;

	status = nrf70_bm_fw_load(rpu_ctx, fw_start, fw_end);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: nrf_wifi_fw_load failed", __func__);
	}

	return status;
}
