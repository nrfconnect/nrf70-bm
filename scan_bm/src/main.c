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

int main(void)
{
    struct nrf70_scan_params scan_params = { 0 };
    int ret;

    LOG_INF("WiFi scan sample application using nRF70 Bare Metal library");

    // Initialize the WiFi module
    CHECK_RET(nrf70_init());

    LOG_INF("Scanning for WiFi networks...");

    // Start scanning for WiFi networks
    CHECK_RET(nrf70_scan_start(&scan_params));

    // Wait for the scan to complete
    while (!nrf70_scan_done())
    {
        // Wait for the scan to complete
    }

    LOG_INF("Scan complete");

    // Print the scan results
    nrf70_scan_print_results();

    // Clean up the WiFi module
    CHECK_RET(nrf70_deinit());

    ret = 0;

cleanup:
    LOG_INF("Exiting WiFi scan sample application with error: %d", ret);
    return ret;
}
