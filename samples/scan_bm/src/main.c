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
#include "nrf70_bm_lib.h"

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

void scan_result_cb(struct nrf70_scan_result *entry)
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

	nrf70_mac_txt(entry->bssid, bssid_str, sizeof(bssid_str));
	printf("%-4d | %-32s | %-4u (%-6s) | %-4d | %-15s | %-17s | %-8s\n",
		   scan_result_cnt, entry->ssid, entry->channel,
		   nrf70_band_txt(entry->band),
		   entry->rssi,
		   nrf70_security_txt(entry->security),
		   bssid_str,
		   nrf70_mfp_txt(entry->mfp));
}
static int prepare_scan_params(struct nrf70_scan_params *params)
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
						(struct nrf70_band_channel *)params->band_chan,
						ARRAY_SIZE(params->band_chan))) {
			printf("Incorrect value(s) in CONFIG_WIFI_SCAN_CHAN_LIST: %s\n",
					CONFIG_WIFI_SCAN_CHAN_LIST);
			return -ENOEXEC;
		}
	}

	return 0;
}

int main(void)
{
	struct nrf70_scan_params scan_params = { 0 };
	int ret;

	printf("WiFi scan sample application using nRF70 Bare Metal library\n");

	// Initialize the WiFi module
	CHECK_RET(nrf70_init());

	printf("Scanning for WiFi networks...\n");

	// Prepare scan parameters
	CHECK_RET(prepare_scan_params(&scan_params));

	while (1)
	{
		// Start scanning for WiFi networks
		CHECK_RET(nrf70_scan_start(&scan_params, scan_result_cb));

		// Wait for the scan to complete or timeout
		unsigned int timeout = 30000;
		while (!is_scan_done && timeout > 0)
		{
			// Wait for the scan to complete
			k_sleep(K_MSEC(500));
			timeout -= 500;
		}

		if (!is_scan_done)
		{
			printf("Scan timeout\n");
		}
		else
		{
			scan_result_cnt = 0;
			printf("Scan complete\n");
		}

		k_sleep(K_MSEC(CONFIG_WIFI_SCAN_INTERVAL_S * 1000));
	}

	// Clean up the WiFi module
	CHECK_RET(nrf70_deinit());

	ret = 0;

cleanup:
	printf("Exiting WiFi scan sample application with error: %d\n", ret);
	return ret;
}
