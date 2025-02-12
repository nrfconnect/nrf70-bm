/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief WiFi scan sample application using nRF70 Bare Metal library.
 */

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

/* Only for k_sleep() */
#include <zephyr/kernel.h>

#include "utils.h"
#include "system/nrf70_bm_lib.h"

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

static int dump_regulatory_info(struct nrf70_bm_regulatory_info *reg_info)
{
	int ret;

	memset(reg_info->chan_info, 0, sizeof(struct nrf70_bm_reg_chan_info) * NRF70_MAX_CHANNELS);
	ret = nrf70_bm_sys_get_reg(reg_info);
	if (ret) {
		printf("Failed to get regulatory info: %d\n", ret);
		return -1;
	}

	/* Dump the regulatory information */
	printf("Regulatory country code: %s\n", reg_info->country_code);
	printf("Number of channels: %d\n", reg_info->num_channels);
	for (int i = 0; i < reg_info->num_channels; i++) {
		printf("Channel %d: center frequency %d MHz, max power %d dBm, "
		       "passive only %d, supported %d, DFS %d\n",
		       i,
		       reg_info->chan_info[i].center_frequency,
		       reg_info->chan_info[i].max_power,
		       reg_info->chan_info[i].passive_only,
		       reg_info->chan_info[i].supported,
		       reg_info->chan_info[i].dfs);
	}

	return 0;
}

int main(void)
{
	struct nrf70_bm_sys_scan_params scan_params = { 0 };
	struct nrf70_bm_regulatory_info reg_info = { 0 };
	int ret;
	int scan_count = 1;

	printf("WiFi scan sample application using nRF70 Bare Metal library\n");

	memcpy(reg_info.country_code, CONFIG_WIFI_SCAN_REG_DOMAIN, 2);
	reg_info.force = true;

	reg_info.chan_info = malloc(sizeof(struct nrf70_bm_reg_chan_info) * NRF70_MAX_CHANNELS);
	if (!reg_info.chan_info) {
		printf("Failed to allocate memory for regulatory info\n");
		ret = -ENOMEM;
		goto cleanup;
	}

	/* Initialize the WiFi module */
	CHECK_RET(nrf70_bm_sys_init(NULL, &reg_info));

	printf("Scanning for WiFi networks...\n");
	/* Prepare scan parameters */
	CHECK_RET(prepare_scan_params(&scan_params));

	while (1)
	{
		/* Start scanning for WiFi networks */
		is_scan_done = false;
		printf("Starting %d scan...\n", scan_count);
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

		if (scan_count % CONFIG_WIFI_SCAN_REG_DOMAIN_SWITCH_COUNT == 0) {
			/* Switch regulatory domain */
			const char *new_domain = (strncmp(reg_info.country_code, CONFIG_WIFI_SCAN_REG_DOMAIN, 2) == 0) ?
									 CONFIG_WIFI_SCAN_REG_DOMAIN_2 : CONFIG_WIFI_SCAN_REG_DOMAIN;
			memcpy(reg_info.country_code, new_domain, 2);
			reg_info.force = true;
			printf("Switching to regulatory domain: %s\n", reg_info.country_code);
			CHECK_RET(nrf70_bm_sys_set_reg(&reg_info));
			CHECK_RET(dump_regulatory_info(&reg_info));
		}

		scan_count++;
		k_sleep(K_MSEC(CONFIG_WIFI_SCAN_INTERVAL_S * 1000));
	}

	/* Clean up the WiFi module */
	CHECK_RET(nrf70_bm_sys_deinit());

	ret = 0;

cleanup:
	free(reg_info.chan_info);
	printf("Exiting WiFi scan sample application with error: %d\n", ret);
	return ret;
}
