/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief nRF70 Bare Metal initialization.
 */
#include "common/nrf70_bm_core.h"
#include "util.h"
#include "system/fmac_api.h"
#include "common/fmac_util.h"

#ifdef CONFIG_NRF70_BOARD_TYPE_DK
#include "common/nrf70_bm_tx_pwr_ceil_dk.h"
#elif CONFIG_NRF70_BOARD_TYPE_EK
#include "common/nrf70_bm_tx_pwr_ceil_ek.h"
#else
#error "Please prepare tx power ceiling header file for your board"
#endif

enum nrf_wifi_status nrf70_bm_fw_load(void *rpu_ctx,
				      const uint8_t *fw_start,
				      const uint8_t *fw_end)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	struct nrf_wifi_fmac_fw_info fw_info = { 0 };

	/* Load the FW patches to the RPU */
	status = nrf_wifi_fmac_fw_parse(rpu_ctx, fw_start, fw_end - fw_start, &fw_info);

	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: nrf_wifi_fmac_fw_parse failed", __func__);
	}

	status = nrf_wifi_fmac_fw_load(rpu_ctx, &fw_info);

	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: nrf_wifi_fmac_fw_load failed", __func__);
	}

	return status;
}

void nrf70_bm_conf_tx_pwr_settings(struct nrf_wifi_tx_pwr_ctrl_params *tx_pwr_ctrl_params,
				   struct nrf_wifi_tx_pwr_ceil_params *tx_pwr_ceil_params)
{
	tx_pwr_ctrl_params->ant_gain_2g = CONFIG_NRF70_ANT_GAIN_2G;
	tx_pwr_ctrl_params->ant_gain_5g_band1 = CONFIG_NRF70_ANT_GAIN_5G_BAND1;
	tx_pwr_ctrl_params->ant_gain_5g_band2 = CONFIG_NRF70_ANT_GAIN_5G_BAND2;
	tx_pwr_ctrl_params->ant_gain_5g_band3 = CONFIG_NRF70_ANT_GAIN_5G_BAND3;
	tx_pwr_ctrl_params->band_edge_2g_lo_dss = CONFIG_NRF70_BAND_2G_LOWER_EDGE_BACKOFF_DSSS;
	tx_pwr_ctrl_params->band_edge_2g_lo_ht = CONFIG_NRF70_BAND_2G_LOWER_EDGE_BACKOFF_HT;
	tx_pwr_ctrl_params->band_edge_2g_lo_he = CONFIG_NRF70_BAND_2G_LOWER_EDGE_BACKOFF_HE;
	tx_pwr_ctrl_params->band_edge_2g_hi_dsss = CONFIG_NRF70_BAND_2G_UPPER_EDGE_BACKOFF_DSSS;
	tx_pwr_ctrl_params->band_edge_2g_hi_ht = CONFIG_NRF70_BAND_2G_UPPER_EDGE_BACKOFF_HT;
	tx_pwr_ctrl_params->band_edge_2g_hi_he = CONFIG_NRF70_BAND_2G_UPPER_EDGE_BACKOFF_HE;
	tx_pwr_ctrl_params->band_edge_5g_unii_1_lo_ht =
		CONFIG_NRF70_BAND_UNII_1_LOWER_EDGE_BACKOFF_HT;
	tx_pwr_ctrl_params->band_edge_5g_unii_1_lo_he =
		CONFIG_NRF70_BAND_UNII_1_LOWER_EDGE_BACKOFF_HE;
	tx_pwr_ctrl_params->band_edge_5g_unii_1_hi_ht =
		CONFIG_NRF70_BAND_UNII_1_UPPER_EDGE_BACKOFF_HT;
	tx_pwr_ctrl_params->band_edge_5g_unii_1_hi_he =
		CONFIG_NRF70_BAND_UNII_1_UPPER_EDGE_BACKOFF_HE;
	tx_pwr_ctrl_params->band_edge_5g_unii_2a_lo_ht =
		CONFIG_NRF70_BAND_UNII_2A_LOWER_EDGE_BACKOFF_HT;
	tx_pwr_ctrl_params->band_edge_5g_unii_2a_lo_he =
		CONFIG_NRF70_BAND_UNII_2A_LOWER_EDGE_BACKOFF_HE;
	tx_pwr_ctrl_params->band_edge_5g_unii_2a_hi_ht =
		CONFIG_NRF70_BAND_UNII_2A_UPPER_EDGE_BACKOFF_HT;
	tx_pwr_ctrl_params->band_edge_5g_unii_2a_hi_he =
		CONFIG_NRF70_BAND_UNII_2A_UPPER_EDGE_BACKOFF_HE;
	tx_pwr_ctrl_params->band_edge_5g_unii_2c_lo_ht =
		CONFIG_NRF70_BAND_UNII_2C_LOWER_EDGE_BACKOFF_HT;
	tx_pwr_ctrl_params->band_edge_5g_unii_2c_lo_he =
		CONFIG_NRF70_BAND_UNII_2C_LOWER_EDGE_BACKOFF_HE;
	tx_pwr_ctrl_params->band_edge_5g_unii_2c_hi_ht =
		CONFIG_NRF70_BAND_UNII_2C_UPPER_EDGE_BACKOFF_HT;
	tx_pwr_ctrl_params->band_edge_5g_unii_2c_hi_he =
		CONFIG_NRF70_BAND_UNII_2C_UPPER_EDGE_BACKOFF_HE;
	tx_pwr_ctrl_params->band_edge_5g_unii_3_lo_ht =
		CONFIG_NRF70_BAND_UNII_3_LOWER_EDGE_BACKOFF_HT;
	tx_pwr_ctrl_params->band_edge_5g_unii_3_lo_he =
		CONFIG_NRF70_BAND_UNII_3_LOWER_EDGE_BACKOFF_HE;
	tx_pwr_ctrl_params->band_edge_5g_unii_3_hi_ht =
		CONFIG_NRF70_BAND_UNII_3_UPPER_EDGE_BACKOFF_HT;
	tx_pwr_ctrl_params->band_edge_5g_unii_3_hi_he =
		CONFIG_NRF70_BAND_UNII_3_UPPER_EDGE_BACKOFF_HE;
	tx_pwr_ctrl_params->band_edge_5g_unii_4_lo_ht =
		CONFIG_NRF70_BAND_UNII_4_LOWER_EDGE_BACKOFF_HT;
	tx_pwr_ctrl_params->band_edge_5g_unii_4_lo_he =
		CONFIG_NRF70_BAND_UNII_4_LOWER_EDGE_BACKOFF_HE;
	tx_pwr_ctrl_params->band_edge_5g_unii_4_hi_ht =
		CONFIG_NRF70_BAND_UNII_4_UPPER_EDGE_BACKOFF_HT;
	tx_pwr_ctrl_params->band_edge_5g_unii_4_hi_he =
		CONFIG_NRF70_BAND_UNII_4_UPPER_EDGE_BACKOFF_HE;

	/* Set power ceiling parameters */
	tx_pwr_ceil_params->max_pwr_2g_dsss = MAX_PWR_2G_DSSS;
	tx_pwr_ceil_params->max_pwr_2g_mcs7 = MAX_PWR_2G_MCS7;
	tx_pwr_ceil_params->max_pwr_2g_mcs0 = MAX_PWR_2G_MCS0;

	#ifndef CONFIG_NRF70_2_4G_ONLY
	tx_pwr_ceil_params->max_pwr_5g_low_mcs7 = MAX_PWR_5G_LOW_MCS7;
	tx_pwr_ceil_params->max_pwr_5g_mid_mcs7 = MAX_PWR_5G_MID_MCS7;
	tx_pwr_ceil_params->max_pwr_5g_high_mcs7 = MAX_PWR_5G_HIGH_MCS7;
	tx_pwr_ceil_params->max_pwr_5g_low_mcs0 = MAX_PWR_5G_LOW_MCS0;
	tx_pwr_ceil_params->max_pwr_5g_mid_mcs0 = MAX_PWR_5G_MID_MCS0;
	tx_pwr_ceil_params->max_pwr_5g_high_mcs0 = MAX_PWR_5G_HIGH_MCS0;
	#endif /* CONFIG_NRF70_2_4G_ONLY */
}

void nrf70_bm_conf_board_dep_params(struct nrf_wifi_board_params *board_params)
{
	board_params->pcb_loss_2g = CONFIG_NRF70_PCB_LOSS_2G;
#ifndef CONFIG_NRF70_2_4G_ONLY
	board_params->pcb_loss_5g_band1 = CONFIG_NRF70_PCB_LOSS_5G_BAND1;
	board_params->pcb_loss_5g_band2 = CONFIG_NRF70_PCB_LOSS_5G_BAND2;
	board_params->pcb_loss_5g_band3 = CONFIG_NRF70_PCB_LOSS_5G_BAND3;
#endif /* CONFIG_NRF70_2_4G_ONLY */
}