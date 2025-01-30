
/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief nRF7002 DK TX power ceiling configuration.
 *
 * The configuration applies to the nRF7002 DK board targets, as well as
 * to board targets for nRF7001 and nRF7000 IC variants emulated on the
 * nRF7002 DK.
 *
 * The following macros are normally auto-generated from DeviceTree
 * when building the nRF70 driver in a Zephyr environment. They are
 * copied here as standard preprocessor directives for ease of porting.
 *
 * The values are represented in 0.25 dB increments.The values for the
 * other MCS rates are interpolated from the values for MCS0 and MCS7.
 *
 *
 * @addtogroup nrf70_bm_txpower nRF70 Bare Metal library
 */

#ifndef NRF70_BM_TX_PWR_CEIL_DK_H__
#define NRF70_BM_TX_PWR_CEIL_DK_H__

/** Maximum power for 2.4 GHz DSSS. */
#define MAX_PWR_2G_DSSS 0x54
/** Maximum power for 2.4 GHz MCS0. */
#define MAX_PWR_2G_MCS0 0x40
/** Maximum power for 2.4 GHz MCS7. */
#define MAX_PWR_2G_MCS7 0x40
/** Maximum power for 5 GHz low band (5150-5330 MHz) MCS0. */
#define MAX_PWR_5G_LOW_MCS0 0x24
/** Maximum power for 5 GHz low band (5150-5330 MHz) MCS7. */
#define MAX_PWR_5G_LOW_MCS7 0x24
/** Maximum power for 5 GHz mid band (5330-5670 MHz) MCS0. */
#define MAX_PWR_5G_MID_MCS0 0x2C
/** Maximum power for 5 GHz mid band (5330-5670 MHz) MCS7. */
#define MAX_PWR_5G_MID_MCS7 0x2C
/** Maximum power for 5 GHz high band (5670-5895 MHz) MCS0. */
#define MAX_PWR_5G_HIGH_MCS0 0x34
/** Maximum power for 5 GHz high band (5670-5895 MHz) MCS7. */
#define MAX_PWR_5G_HIGH_MCS7 0x34

#endif /* NRF70_BM_TX_PWR_CEIL_DK_H__ */
