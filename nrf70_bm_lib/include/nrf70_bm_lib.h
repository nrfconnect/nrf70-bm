/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief nRF70 Bare Metal library.
 * @defgroup nrf70_bm nRF70 Bare Metal library
 */

#ifndef NRF70_BM_H__
#define NRF70_BM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/** @brief Log levels */
#if CONFIG_NRF70_BM_LOG_LEVEL > 0
#define NRF70_LOG_ERR(...) printf(__VA_ARGS__); printf("\n")
#else
#define NRF70_LOG_ERR(...)
#endif

#if CONFIG_NRF70_BM_LOG_LEVEL > 1
#define NRF70_LOG_WRN(...) printf(__VA_ARGS__); printf("\n")
#else
#define NRF70_LOG_WRN(...)
#endif

#if CONFIG_NRF70_BM_LOG_LEVEL > 2
#define NRF70_LOG_INF(...) printf(__VA_ARGS__); printf("\n")
#else
#define NRF70_LOG_INF(...)
#endif

#if CONFIG_NRF70_BM_LOG_LEVEL > 3
#define NRF70_LOG_DBG(...) printf(__VA_ARGS__); printf("\n")
#else
#define NRF70_LOG_DBG(...)
#endif


#define NR70_SCAN_SSID_MAX_LEN 32
#ifdef CONFIG_NRF70_SCAN_SSID_FILT_MAX
#define NRF70_SCAN_SSID_FILT_MAX CONFIG_NRF70_SCAN_SSID_FILT_MAX
#else
#define NRF70_SCAN_SSID_FILT_MAX 1
#endif /* CONFIG_NRF70_SCAN_SSID_FILT_MAX */

#ifdef CONFIG_NRF70_SCAN_CHAN_MAX_MANUAL
#define NRF70_SCAN_CHAN_MAX_MANUAL CONFIG_NRF70_SCAN_CHAN_MAX_MANUAL
#else
#define NRF70_SCAN_CHAN_MAX_MANUAL 1
#endif /* CONFIG_NRF70_SCAN_CHAN_MAX_MANUAL */

/** @brief Wi-Fi version */
struct wifi_version {
	/** Driver version */
	const char *drv_version;
	/** Firmware version */
	const char *fw_version;
};

/** @brief Wi-Fi scanning types. */
enum nrf70_scan_type {
	/** Active scanning (default). */
	WIFI_SCAN_TYPE_ACTIVE = 0,
	/** Passive scanning. */
	WIFI_SCAN_TYPE_PASSIVE,
};

/**
 * @brief Wi-Fi structure to uniquely identify a band-channel pair
 */
struct wifi_band_channel {
	/** Frequency band */
	uint8_t band;
	/** Channel */
	uint8_t channel;
};

/**
 * @brief Wi-Fi scan parameters structure.
 * Used to specify parameters which can control how the Wi-Fi scan
 * is performed.
 */
struct nrf70_scan_params {
	/** Scan type, see enum wifi_scan_type.
	 *
	 * The scan_type is only a hint to the underlying Wi-Fi chip for the
	 * preferred mode of scan. The actual mode of scan can depend on factors
	 * such as the Wi-Fi chip implementation support, regulatory domain
	 * restrictions etc.
	 */
	enum nrf70_scan_type scan_type;
	/** Bitmap of bands to be scanned.
	 *  Refer to ::wifi_frequency_bands for bit position of each band.
	 */
	uint8_t bands;
	/** Active scan dwell time (in ms) on a channel.
	 */
	uint16_t dwell_time_active;
	/** Passive scan dwell time (in ms) on a channel.
	 */
	uint16_t dwell_time_passive;
	/** Array of SSID strings to scan.
	 */
	const char *ssids[NRF70_SCAN_SSID_FILT_MAX];
	/** Specifies the maximum number of scan results to return. These results would be the
	 * BSSIDS with the best RSSI values, in all the scanned channels. This should only be
	 * used to limit the number of returned scan results, and cannot be counted upon to limit
	 * the scan time, since the underlying Wi-Fi chip might have to scan all the channels to
	 * find the max_bss_cnt number of APs with the best signal strengths. A value of 0
	 * signifies that there is no restriction on the number of scan results to be returned.
	 */
	uint16_t max_bss_cnt;
	/** Channel information array indexed on Wi-Fi frequency bands and channels within that
	 * band.
	 * E.g. to scan channel 6 and 11 on the 2.4 GHz band, channel 36 on the 5 GHz band:
	 * @code{.c}
	 *     chan[0] = {WIFI_FREQ_BAND_2_4_GHZ, 6};
	 *     chan[1] = {WIFI_FREQ_BAND_2_4_GHZ, 11};
	 *     chan[2] = {WIFI_FREQ_BAND_5_GHZ, 36};
	 * @endcode
	 *
	 *  This list specifies the channels to be __considered for scan__. The underlying
	 *  Wi-Fi chip can silently omit some channels due to various reasons such as channels
	 *  not conforming to regulatory restrictions etc. The invoker of the API should
	 *  ensure that the channels specified follow regulatory rules.
	 */
	struct wifi_band_channel band_chan[NRF70_SCAN_CHAN_MAX_MANUAL];
};

/**@brief Initialize the WiFi module.
 *
 * @retval 0 If the operation was successful.
 * @retval -1 If the operation failed.
 */
int nrf70_init(void);

/**@brief Start scanning for WiFi networks.
 * 
 * @param[in] scan_params Scan parameters.
 * 
 * @retval 0 If the operation was successful.
 * @retval -EINVAL If the scan parameters are invalid.
 * @retval -EBUSY If the scan is already in progress.
 * @retval -EIO If the operation failed.
 * @retval -ENOMEM If there is not enough memory to start the scan.
 */
int nrf70_scan_start(struct nrf70_scan_params *scan_params);

/**@brief Check if the WiFi scan is done.
 *
 * @retval 0 If the scan is not done.
 * @retval 1 If the scan is done.
 */
bool nrf70_scan_done(void);

/**@brief Print the WiFi scan results.
 */
void nrf70_scan_print_results(void);

/**@brief Clean up the WiFi module.
 */
int nrf70_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* NRF70_BM_H__ */
