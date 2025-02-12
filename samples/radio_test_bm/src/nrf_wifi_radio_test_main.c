/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>

#include "radio_test/nrf70_bm_lib.h"
#include "nrf_wifi_radio_test_shell.h"

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

int main(void)
{
	int ret = 0;
	struct nrf70_bm_regulatory_info reg_info = { 0 };

#ifndef CONFIG_ZEPHYR_SHELL
#error "This sample application requires shell support, please enable CONFIG_ZEPHYR_SHELL"
#endif

	memcpy(reg_info.country_code, CONFIG_WIFI_RT_REG_DOMAIN, 2);
	reg_info.force = true;

	reg_info.chan_info = malloc(sizeof(struct nrf70_bm_reg_chan_info) * NRF70_MAX_CHANNELS);
	if (!reg_info.chan_info) {
		printf("Failed to allocate memory for regulatory info\n");
		ret = -ENOMEM;
		goto cleanup;
	}

	/* Initialize the Wi-Fi module */
	CHECK_RET(nrf70_bm_rt_init(&reg_info));
	printf("Initialized WiFi module, ready for radio test\n");

	ret = nrf_wifi_radio_test_shell_init();
	if (ret) {
		printf("Failed to initialize radio test: %d\n", ret);
		goto cleanup;
	}

cleanup:
	if (reg_info.chan_info) {
		free(reg_info.chan_info);
	}

	if (ret) {
		nrf70_bm_rt_deinit();
		printf("Exiting WiFi radio test sample application with error: %d\n", ret);
	}
	return ret;
}
