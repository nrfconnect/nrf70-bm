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
#include <zephyr/init.h>
#include <ctype.h>
#include <nrf70_bm_core.h>
#include <host_rpu_sys_if.h>
#include <fmac_structs.h>

#define NRF_WIFI_RADIO_TEST_INIT_TIMEOUT_MS 5000
#ifdef CONFIG_ZEPHYR_SHELL
#include <shell/shell.h>
#include <shell/shell_uart.h>

extern struct shell *shell_global;

#define RT_SHELL_PRINTF_INFO(fmt, ...)                                         \
	do {                                                                         \
		if (shell_global) {                                                        \
			shell_fprintf(shell_global, SHELL_INFO, fmt, ##__VA_ARGS__);             \
		} else {                                                                   \
			printf(fmt, ##__VA_ARGS__);                                              \
		}                                                                          \
	} while (0)

#define RT_SHELL_PRINTF_ERROR(fmt, ...)                                        \
	do {                                                                         \
		if (shell_global) {                                                        \
			shell_fprintf(shell_global, SHELL_ERROR, fmt, ##__VA_ARGS__);            \
		} else {                                                                   \
			printf(fmt, ##__VA_ARGS__);                                              \
		}                                                                          \
	} while (0)

#define RT_SHELL_PRINTF_WARNING(fmt, ...)                                      \
	do {                                                                         \
		if (shell_global) {                                                        \
			shell_fprintf(shell_global, SHELL_WARNING, fmt, ##__VA_ARGS__);          \
		} else {                                                                   \
			printf(fmt, ##__VA_ARGS__);                                              \
		}                                                                          \
	} while (0)

#define DEFINE_CMD_HANDLER(name)                                               \
  static int name##_sh(const struct shell *sh, size_t argc, const char *argv[]) {           \
    shell_global = (struct shell *)sh;                                         \
    return name(argc, argv);                                                   \
  }

#define RTSH(fn) fn##_sh

#else
#define RT_SHELL_PRINTF_INFO(fmt, ...) printf(fmt, ##__VA_ARGS__)

#define RT_SHELL_PRINTF_ERROR(fmt, ...) printf(fmt, ##__VA_ARGS__)

#define RT_SHELL_PRINTF_WARNING(fmt, ...) printf(fmt, ##__VA_ARGS__)

#define DEFINE_CMD_HANDLER(name)
#endif

struct nrf_wifi_ctx_zep_rt {
	struct nrf_wifi_fmac_priv *fmac_priv;
	struct rpu_conf_params conf_params;
};

int nrf_wifi_radio_test_shell_init(void);

#endif /* NRF_WIFI_RADIO_TEST_SHELL_H__ */
