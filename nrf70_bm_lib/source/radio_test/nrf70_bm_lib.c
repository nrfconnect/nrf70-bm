/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief nRF70 Bare Metal library.
 */

#include "radio_test/nrf70_bm_lib.h"
#include "util.h"

int nrf70_bm_rt_init(void)
{
	int ret;

	// Initialize the WiFi module
	ret = nrf70_bm_rt_fmac_init();
	if (ret) {
		NRF70_LOG_ERR("Failed to initialize FMAC module");
		goto err;
	}

	return 0;
err:
	return ret;
}


int nrf70_bm_rt_deinit(void)
{
	int ret = -1;

	// Clean up the WiFi module
	ret = nrf70_bm_rt_fmac_deinit();
	if (ret) {
		NRF70_LOG_ERR("Failed to deinitialize FMAC module");
		goto err;
	}

	return 0;
err:
	return ret;
}