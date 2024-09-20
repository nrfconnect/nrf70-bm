/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>

#include "nrf70_bm_lib.h"
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

#ifndef CONFIG_ZEPHYR_SHELL
#error "This sample application requires shell support, please enable CONFIG_ZEPHYR_SHELL"
#endif

	// Initialize the WiFi module
	CHECK_RET(nrf70_init());

	printf("Initialized WiFi module, ready for radio test\n");

	nrf_wifi_radio_test_shell_init();

cleanup:
	printf("Exiting WiFi radio test sample application with error: %d\n", ret);
	return ret;
}
