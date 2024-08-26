/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/* @file
 * @brief nRF Wi-Fi radio-test mode shell module
 */
#ifndef NRF_WIFI_RADIO_TEST_SHELL_H__
#define NRF_WIFI_RADIO_TEST_SHELL_H__
#include <zephyr/kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/shell/shell.h>
#include <zephyr/init.h>
#include <ctype.h>
#include <nrf70_bm_core.h>
#include <host_rpu_sys_if.h>
#include <fmac_structs.h>

struct nrf_wifi_ctx_zep_rt {
	struct nrf_wifi_fmac_priv *fmac_priv;
	struct rpu_conf_params conf_params;
};

int nrf_wifi_radio_test_shell_init(void);

#endif /* NRF_WIFI_RADIO_TEST_SHELL_H__ */
