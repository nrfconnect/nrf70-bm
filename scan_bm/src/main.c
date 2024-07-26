/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief WiFi scan sample application using nRF70 Bare Metal library.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>

LOG_MODULE_REGISTER(nrf70_scan_bm, CONFIG_NR70_SCAN_BM_SAMPLE_LOG_LEVEL);

#include "nrf70_bm_lib.h"

#define CHECK_RET(func) do { \
	ret = func; \
	if (ret) { \
		printk("Error: %d\n", ret); \
		goto cleanup; \
	} \
} while (0)

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

int main(void)
{
	//struct nrf70_scan_params scan_params = { 0 };
	int ret;

	LOG_INF("WiFi scan sample application using nRF70 Bare Metal library");

	// Initialize the WiFi module
	CHECK_RET(nrf70_init());

	LOG_INF("Scanning for WiFi networks...");

	while (1)
	{
		// Start scanning for WiFi networks
		CHECK_RET(nrf70_scan_start(NULL, scan_result_cb));

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
			LOG_INF("Scan timeout");
		}
		else
		{
			scan_result_cnt = 0;
			LOG_INF("Scan complete");
		}

		k_sleep(K_SECONDS(5));
	}

	// Clean up the WiFi module
	CHECK_RET(nrf70_deinit());

	ret = 0;

cleanup:
	LOG_INF("Exiting WiFi scan sample application with error: %d", ret);
	return ret;
}
