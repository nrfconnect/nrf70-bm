/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief nRF70 Bare Metal library.
 * @defgroup nrf70_bm nRF70 Bare Metal library
 */

#ifndef NRF70_BM_RT_LIB_H__
#define NRF70_BM_RT_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "radio_test/nrf70_bm_core.h"

extern struct nrf70_bm_rt_wifi_drv_priv nrf70_bm_rt_priv;

/**@brief Initialize the WiFi module in the radio test mode of operation.
 *
 * This function initializes the nRF70 device and prepares it for
 * operation in the radio test mode.
 * This also includes powering up the device and setting up the necessary
 * configurations including the download of the firmware patch for nRF70 device.
 *
 * @param[in] mac_addr MAC address of the device.
 * @param[in] reg_info Regulatory information.
 *
 * @retval 0 If the operation was successful.
 * @retval -1 If the operation failed.
 */
int nrf70_bm_rt_init(struct nrf70_bm_regulatory_info *reg_info);

/**@brief Clean up the WiFi module in the radio test mode of operation.
 *
 * This function de-initializes the nRF70 device, operating in the
 * radio test mode, free up resources and powers it down.
 * Any further operations on the device will require re-initialization.
 *
 * @retval 0 If the operation was successful.
 */
int nrf70_bm_rt_deinit(void);

/** @brief Set nRF70 regulatory information.
 *
 * This function sets the regulatory information of the nRF70 device.
 * The regulatory information includes the country code, the number of channels
 * supported and the channel information.
 *
 * @param[in] reg_info Regulatory information.
 *
 * @retval 0 If the operation was successful.
 * @retval -1 If the operation failed.
 */
int nrf70_bm_rt_set_reg(struct nrf70_bm_regulatory_info *reg_info);

/** @brief Get nRF70 regulatory information.
 *
 * This function retrieves the regulatory information of the nRF70 device.
 * The regulatory information includes the country code, the number of channels
 * supported and the channel information.
 *
 * @param[out] reg_info Regulatory information.
 *
 * @retval 0 If the operation was successful.
 * @retval -1 If the operation failed. *
 */
int nrf70_bm_rt_get_reg(struct nrf70_bm_regulatory_info *reg_info);

#ifdef __cplusplus
}
#endif

#endif /* NRF70_BM_H__ */
