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

int nrf70_bm_rt_init(struct nrf70_bm_regulatory_info *reg_info)
{
	int ret;
	struct nrf70_bm_regulatory_info reg_info_curr = { 0 };

	// Initialize the WiFi module
	ret = nrf70_bm_rt_fmac_init();
	if (ret) {
		NRF70_LOG_ERR("Failed to initialize FMAC module");
		goto out;
	}

	reg_info_curr.chan_info = malloc(sizeof(struct nrf70_bm_reg_chan_info) * NRF70_MAX_CHANNELS);
	if (!reg_info_curr.chan_info) {
		printf("Failed to allocate memory for regulatory info\n");
		ret = -1;
		goto deinit;
	}

	ret = nrf70_bm_rt_get_reg(&reg_info_curr);
	if (ret) {
		NRF70_LOG_ERR("Failed to get regulatory info");
		goto deinit;
	}

	printf("Current regulatory country code: %s\n",
	       reg_info_curr.country_code);

	if (reg_info) {
		printf("Setting regulatory country code to: %s\n",
		       reg_info->country_code);
		ret = nrf70_bm_rt_fmac_set_reg(reg_info);
		if (ret) {
			NRF70_LOG_ERR("Failed to set regulatory info");
			goto deinit;
		}

		memset(reg_info_curr.chan_info,
		       0,
		       sizeof(struct nrf70_bm_reg_chan_info) * NRF70_MAX_CHANNELS);

		ret = nrf70_bm_rt_get_reg(&reg_info_curr);
		if (ret) {
			printf("Failed to get regulatory info: %d\n", ret);
			return -1;
		}

		printf("Regulatory country code set to: %s\n", reg_info_curr.country_code);
	}

	ret = 0;
	goto out;
deinit:
	nrf70_bm_rt_fmac_deinit();
out:
	if (reg_info_curr.chan_info) {
		free(reg_info_curr.chan_info);
	}
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

int nrf70_bm_rt_set_reg(struct nrf70_bm_regulatory_info *reg_info)
{
	return nrf70_bm_rt_fmac_set_reg(reg_info);
}

int nrf70_bm_rt_get_reg(struct nrf70_bm_regulatory_info *reg_info)
{
	return nrf70_bm_rt_fmac_get_reg(reg_info);
}