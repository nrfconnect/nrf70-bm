/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief nRF70 Bare Metal library.
 */

#include "nrf70_bm_lib.h"
#include "nrf70_bm_init.h"

int nrf70_init(void)
{
	int ret;

	// Initialize the WiFi module
	ret = nrf70_fmac_init();
	if (ret) {
		NRF70_LOG_ERR("Failed to initialize FMAC module\n");
		goto err;
	}

	return 0;
err:
	return ret;
}

int nrf70_scan_start(struct nrf70_scan_params *scan_params)
{
	// Start scanning for WiFi networks

	return 0;
}

bool nrf70_scan_done(void)
{
	// Wait for the scan to complete
	return 0;
}

void nrf70_scan_print_results(void)
{
	// Print the scan results
}

int nrf70_deinit(void)
{
	int
	// Clean up the WiFi module
	ret = nrf70_fmac_deinit();
	if (ret) {
		NRF70_LOG_ERR("Failed to deinitialize FMAC module\n");
		goto err;
	}

	return 0;
err:
	return ret;
}

