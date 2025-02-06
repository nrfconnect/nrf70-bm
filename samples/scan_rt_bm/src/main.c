/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief WiFi scan and radio test combo sample application using nRF70 Bare Metal library.
 */

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

/* Only for k_sleep() */
#include <zephyr/kernel.h>
#include "system/nrf70_bm_lib.h"
#include "radio_test/nrf70_bm_lib.h"
#include "utils.h"

extern struct nrf70_bm_rt_wifi_drv_priv nrf70_bm_rt_priv;
struct nrf70_bm_rt_wifi_ctx *ctx = &nrf70_bm_rt_priv.rpu_ctx_bm;

#define CHECK_RET(func) do { \
	ret = func; \
	if (ret) { \
		printf("Error: %d\n", ret); \
		goto cleanup; \
	} \
} while (0)

bool debug_enabled;

#define debug_print(fmt, ...) \
	do {if (debug_enabled) printf(fmt, ##__VA_ARGS__); } while (0)

unsigned int scan_result_cnt;
bool is_scan_done;

void scan_result_cb(struct nrf70_bm_sys_scan_result *entry)
{
	unsigned char bssid_str[18];

	if (!entry)
	{
		is_scan_done = true;
		return;
	}

	if (++scan_result_cnt == 1) {
		printf("\n%-4s | %-32s | %-13s | %-4s | %-15s | %-17s | %-8s\n",
		   "Num", "SSID", "Chan (Band)", "RSSI", "Security", "BSSID", "MFP");
	}

	nrf70_bm_sys_mac_txt(entry->bssid, bssid_str, sizeof(bssid_str));
	printf("%-4d | %-32s | %-4u (%-6s) | %-4d | %-15s | %-17s | %-8s\n",
		   scan_result_cnt, entry->ssid, entry->channel,
		   nrf70_bm_sys_band_txt(entry->band),
		   entry->rssi,
		   nrf70_bm_sys_security_txt(entry->security),
		   bssid_str,
		   nrf70_bm_sys_mfp_txt(entry->mfp));
}
static int prepare_scan_params(struct nrf70_bm_sys_scan_params *params)
{
	int band_str_len = sizeof(CONFIG_WIFI_SCAN_BANDS_LIST);

	if (band_str_len - 1) {
		char *buf = malloc(band_str_len);

		if (!buf) {
			printf("Malloc Failed\n");
			return -EINVAL;
		}
		strcpy(buf, CONFIG_WIFI_SCAN_BANDS_LIST);
		if (wifi_utils_parse_scan_bands(buf, &params->bands)) {
			printf("Incorrect value(s) in CONFIG_WIFI_SCAN_BANDS_LIST: %s\n",
					CONFIG_WIFI_SCAN_BANDS_LIST);
			free(buf);
			return -ENOEXEC;
		}
		free(buf);
	}

	if (sizeof(CONFIG_WIFI_SCAN_CHAN_LIST) - 1) {
		if (wifi_utils_parse_scan_chan(CONFIG_WIFI_SCAN_CHAN_LIST,
						(struct nrf70_bm_sys_band_channel *)params->band_chan,
						ARRAY_SIZE(params->band_chan))) {
			printf("Incorrect value(s) in CONFIG_WIFI_SCAN_CHAN_LIST: %s\n",
					CONFIG_WIFI_SCAN_CHAN_LIST);
			return -ENOEXEC;
		}
	}

	params->dwell_time_passive = CONFIG_WIFI_SCAN_DWELL_TIME_PASSIVE;
	params->dwell_time_active = CONFIG_WIFI_SCAN_DWELL_TIME_ACTIVE;

	if (IS_ENABLED(CONFIG_WIFI_SCAN_TYPE_PASSIVE)) {
		params->scan_type = NRF70_SCAN_TYPE_PASSIVE;
	} else{
		params->scan_type = NRF70_SCAN_TYPE_ACTIVE;
	}

	return 0;
}

static int nrf70_bm_rt_conf_init(struct rpu_conf_params *conf_params)
{
	int ret = -1;
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	unsigned char country_code[NRF_WIFI_COUNTRY_CODE_LEN] = { 0 };

	/* Check and save regulatory country code currently set */
	if (strlen(conf_params->country_code)) {
	  memcpy(country_code, conf_params->country_code, NRF_WIFI_COUNTRY_CODE_LEN);
	}

	memset(conf_params, 0, sizeof(*conf_params));

	/* Initialize values which are other than 0 */
	conf_params->op_mode = RPU_OP_MODE_RADIO_TEST;

	status = nrf_wifi_rt_fmac_rf_params_get(ctx->rpu_ctx,
						(struct nrf_wifi_phy_rf_params *)conf_params->rf_params);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
	  goto out;
	}

	conf_params->tx_pkt_nss = 1;
	conf_params->tx_pkt_gap_us = 0;
	conf_params->tx_power = MAX_TX_PWR_SYS_TEST;
	conf_params->chan.primary_num = 1;
	conf_params->tx_mode = 1;
	conf_params->tx_pkt_num = -1;
	conf_params->tx_pkt_len = 1400;
	conf_params->tx_pkt_preamble = 0;
	conf_params->tx_pkt_rate = 6;
	conf_params->he_ltf = 2;
	conf_params->he_gi = 2;
	conf_params->aux_adc_input_chain_id = 1;
	conf_params->ru_tone = 26;
	conf_params->ru_index = 1;
	conf_params->tx_pkt_cw = 15;
	conf_params->phy_calib = NRF_WIFI_DEF_PHY_CALIB;

	/* Store back the currently set country code */
	if (strlen(country_code)) {
		memcpy(conf_params->country_code,
		       country_code,
		       NRF_WIFI_COUNTRY_CODE_LEN);
	} else {
		memcpy(conf_params->country_code,
		       "00",
		       NRF_WIFI_COUNTRY_CODE_LEN);
	}

	ret = 0;
out:
	return ret;
}

static int nrf_wifi_radio_test_set_tx(struct rpu_conf_params *conf_params,
				      bool start)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;

	if (start) {
		conf_params->tx = 1;
	} else {
		conf_params->tx = 0;
	}
	status = nrf_wifi_rt_fmac_prog_tx(ctx->rpu_ctx, conf_params);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		printf("Programming TX failed\n");
		return -ENOEXEC;
	}

	return 0;
}


static int nrf70_bm_rt_stats_get(void)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	struct rpu_rt_op_stats stats;

	memset(&stats, 0, sizeof(stats));

	status = nrf_wifi_rt_fmac_stats_get(ctx->rpu_ctx,
					    ctx->conf_params.op_mode,
					    &stats);

	if (status != NRF_WIFI_STATUS_SUCCESS) {
		printf("nrf_wifi_fmac_stats_get failed\n");
		return -ENOEXEC;
	}

	printf("************* PHY STATS ***********\n");

	printf("rssi_avg = %d dBm\n",
	       stats.fw.phy.rssi_avg);

	printf("ofdm_crc32_pass_cnt=%d\n",
	       stats.fw.phy.ofdm_crc32_pass_cnt);

	printf("ofdm_crc32_fail_cnt=%d\n",
	       stats.fw.phy.ofdm_crc32_fail_cnt);

	printf("dsss_crc32_pass_cnt=%d\n",
	       stats.fw.phy.dsss_crc32_pass_cnt);

	printf("dsss_crc32_fail_cnt=%d\n",
	       stats.fw.phy.dsss_crc32_fail_cnt);

	return 0;
}


int main(void)
{
	struct nrf70_bm_sys_scan_params scan_params = { 0 };
	struct nrf70_bm_regulatory_info reg_info = { 0 };
	bool sys_init = false;
	bool rt_init = false;
	int ret = -1;

	/*
	 * Radio test operation
	 */
	/* Initialize the WiFi module in the radio test mode of operation */
	printf("\n\nWi-Fi radio test functionality using nRF70 Bare Metal library\n\n");

	CHECK_RET(nrf70_bm_rt_init());
	rt_init = true;

	CHECK_RET(nrf70_bm_rt_conf_init(&ctx->conf_params));

	/* Start transmit*/
	CHECK_RET(nrf_wifi_radio_test_set_tx(&ctx->conf_params, true));

	k_sleep(K_MSEC(5000));

	/* Stop transmit */
	CHECK_RET(nrf_wifi_radio_test_set_tx(&ctx->conf_params, false));

	CHECK_RET(nrf70_bm_rt_stats_get());

	printf("\n\nDeinitializing radio test functionality\n\n");

	nrf70_bm_rt_deinit();
	rt_init = false;

	/*
	 * Scan operation
	 */
	printf("\n\nSwitching to Wi-Fi scan functionality using nRF70 Bare Metal library\n");

	memcpy(reg_info.country_code, CONFIG_WIFI_SCAN_REG_DOMAIN, 2);
	reg_info.force = true;

	/* Initialize the WiFi module in the System mode of operation */
	CHECK_RET(nrf70_bm_sys_init(NULL, &reg_info));
	sys_init = true;

	printf("Scanning for WiFi networks...\n");

	/* Prepare scan parameters */
	CHECK_RET(prepare_scan_params(&scan_params));

	/* Start scanning for WiFi networks */
	is_scan_done = false;

	printf("Starting scan\n");
	CHECK_RET(nrf70_bm_sys_scan_start(&scan_params, scan_result_cb));

	/* Wait for the scan to complete or timeout */
	unsigned int timeout = 30000;
	while (!is_scan_done && timeout > 0) {
		/* Wait for the scan to complete */
		k_sleep(K_MSEC(500));
		timeout -= 500;
	}

	if (!is_scan_done) {
		printf("Scan timeout\n");
	} else {
		scan_result_cnt = 0;
		printf("Scan complete\n");
	}

	/* Clean up the WiFi module in the system mode of operation */
	printf("\n\nDeinitializing scan functionality\n\n");
	nrf70_bm_sys_deinit();
	sys_init = false;

	ret = 0;
cleanup:
	if (sys_init) {
		nrf70_bm_sys_deinit();
		sys_init = false;
	}

	if (rt_init) {
		nrf70_bm_rt_deinit();
		rt_init = false;
	}

	if (ret) {
		printf("Exiting WiFi scan and radio test combo sample application with error: %d\n", ret);
	}

	return ret;
}