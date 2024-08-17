/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief Wi-Fi utility functions.
 */

#include "utils.h"

#define NRF70_MAX_BAND_STR_LEN 3
#define NRF70_MAX_CHAN_STR_LEN 4
#define NRF70_BAND_STR_SIZE_MAX 8
#define NRF70_SCAN_MAX_BSS_CNT 65535

/* Ensure 'strtok_r' is available even with -std=c99. */
char *strtok_r(char *str, const char *delim, char **saveptr);

static const uint8_t valid_5g_chans_20mhz[] = {32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 96, 100,
	104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 149, 153, 157, 159, 161,
	163, 165, 167, 169, 171, 173, 175, 177};

static enum nrf70_frequency_bands wifi_utils_map_band_str_to_idx(char *band_str)
{
	enum nrf70_frequency_bands band = NRF70_FREQ_BAND_UNKNOWN;

	if (!strcmp(band_str, "2")) {
		band = NRF70_FREQ_BAND_2_4_GHZ;
	} else if (!strcmp(band_str, "5")) {
		band = NRF70_FREQ_BAND_5_GHZ;
	} else if (!strcmp(band_str, "6")) {
		band = NRF70_FREQ_BAND_6_GHZ;
	}

	return band;
}


bool wifi_utils_validate_chan_2g(uint16_t chan)
{
	if ((chan >= 1) && (chan <= 14)) {
		return true;
	}

	return false;
}


bool wifi_utils_validate_chan_5g(uint16_t chan)
{
	uint16_t i;

	for (i = 0; i < ARRAY_SIZE(valid_5g_chans_20mhz); i++) {
		if (chan == valid_5g_chans_20mhz[i]) {
			return true;
		}
	}

	return false;
}


bool wifi_utils_validate_chan_6g(uint16_t chan)
{
	if (((chan >= 1) && (chan <= 233) && (!((chan - 1) % 4))) ||
	    (chan == 2)) {
		return true;
	}

	return false;
}


bool wifi_utils_validate_chan(uint8_t band,
			      uint16_t chan)
{
	bool result = false;

	switch (band) {
	case NRF70_FREQ_BAND_2_4_GHZ:
		result = wifi_utils_validate_chan_2g(chan);
		break;
	case NRF70_FREQ_BAND_5_GHZ:
		result = wifi_utils_validate_chan_5g(chan);
		break;
	case NRF70_FREQ_BAND_6_GHZ:
		result = wifi_utils_validate_chan_6g(chan);
		break;
	default:
		printf("Unknown band: %d", band);
		break;
	}

	return result;
}


static int wifi_utils_get_all_chans_in_range(uint8_t chan_start,
		uint8_t chan_end,
		struct nrf70_band_channel *band_chan,
		uint8_t band_idx,
		uint8_t *chan_idx)
{
	uint8_t i;
	bool start = false;
	bool end = false;
	uint8_t idx;

	if (!wifi_utils_validate_chan(band_idx, chan_start)) {
		printf("Invalid channel value %d in band %d", chan_start, band_idx);
		return -EINVAL;
	}

	if (!wifi_utils_validate_chan(band_idx, chan_end)) {
		printf("Invalid channel value %d in band %d", chan_end, band_idx);
		return -EINVAL;
	}

	if (chan_end < chan_start) {
		printf("Channel range end (%d) cannot be less than start (%d)",
			chan_end,
			chan_start);
		return -EINVAL;
	}

	switch (band_idx) {
	case NRF70_FREQ_BAND_2_4_GHZ:
		idx = *chan_idx;

		for (i = chan_start+1; i <= chan_end; i++) {
			band_chan[idx].band = band_idx;
			band_chan[idx].channel = i;
			idx++;
		}

		*chan_idx = idx;

		break;
	case NRF70_FREQ_BAND_5_GHZ:
		idx = *chan_idx;

		for (i = 0; i < ARRAY_SIZE(valid_5g_chans_20mhz); i++) {
			if (valid_5g_chans_20mhz[i] == chan_start) {
				start = true;
				continue;
			}

			if (valid_5g_chans_20mhz[i] == chan_end) {
				end = true;
			}

			if (start) {
				band_chan[idx].band = band_idx;
				band_chan[idx].channel = valid_5g_chans_20mhz[i];
				idx++;
			}

			if (end) {
				*chan_idx = idx;
				break;
			}
		}

		break;
	case NRF70_FREQ_BAND_6_GHZ:
		idx = *chan_idx;

		i = chan_start+1;

		while (i <= chan_end) {
			band_chan[idx].band = band_idx;
			band_chan[idx].channel = i;
			idx++;

			if (i == 1) {
				i++;
			} else if (i == 2) {
				i += 3;
			} else {
				i += 4;
			}
		}

		*chan_idx = idx;
		break;
	default:
		printf("Unknown band value: %d", band_idx);
		return -EINVAL;
	}

	return 0;
}


static int wifi_utils_validate_chan_str(char *chan_str)
{
	uint8_t i;

	if ((!chan_str) || (!strlen(chan_str))) {
		printf("Null or empty channel string\n");
		return -EINVAL;
	}

	for (i = 0; i < strlen(chan_str); i++) {
		if (!isdigit((int)chan_str[i])) {
			printf("Invalid character in channel string %c\n", chan_str[i]);
			return -EINVAL;
		}
	}

	return 0;
}


int wifi_utils_parse_scan_bands(char *scan_bands_str, uint8_t *band_map)
{
	char parse_str[NRF70_BAND_STR_SIZE_MAX + 1];
	char *band_str = NULL;
	char *ctx = NULL;
	enum nrf70_frequency_bands band = NRF70_FREQ_BAND_UNKNOWN;
	int len;

	if (!scan_bands_str) {
		return -EINVAL;
	}

	len = strlen(scan_bands_str);

	if (len > NRF70_BAND_STR_SIZE_MAX) {
		printf("Band string (%s) size (%d) exceeds maximum allowed value (%d)",
			scan_bands_str,
			len,
			NRF70_BAND_STR_SIZE_MAX);
		return -EINVAL;
	}

	strncpy(parse_str, scan_bands_str, sizeof(parse_str) - 1);
	parse_str[sizeof(parse_str) - 1] = '\0';

	band_str = strtok_r(parse_str, ",", &ctx);

	while (band_str) {
		band = wifi_utils_map_band_str_to_idx(band_str);

		if (band == NRF70_FREQ_BAND_UNKNOWN) {
			printf("Invalid band value: %s", band_str);
			return -EINVAL;
		}

		*band_map |= (1 << band);

		band_str = strtok_r(NULL, ",", &ctx);
	}

	return 0;
}

int wifi_utils_parse_scan_ssids(char *scan_ssids_str,
				const char *ssids[],
				uint8_t num_ssids)
{
	int len;

	if (!scan_ssids_str) {
		return -EINVAL;
	}

	len = strlen(scan_ssids_str);
	if (len > NR70_SCAN_SSID_MAX_LEN) {
		printf("SSID string (%s) size (%d) exceeds maximum allowed value (%d)",
			scan_ssids_str,
			len,
			NR70_SCAN_SSID_MAX_LEN);
		return -EINVAL;
	}

	for (int i = 0; i < num_ssids; i++) {
		if (ssids[i] != NULL) {
			continue;
		}
		ssids[i] = scan_ssids_str;
		return 0;
	}

	printf("Exceeded maximum allowed SSIDs (%d)", num_ssids);
	return 0;
}


int wifi_utils_parse_scan_chan(char *scan_chan_str,
			       struct nrf70_band_channel *band_chan,
			       uint8_t max_channels)
{
	char band_str[NRF70_MAX_BAND_STR_LEN] = {0};
	char chan_str[NRF70_MAX_CHAN_STR_LEN] = {0};
	enum nrf70_frequency_bands band = NRF70_FREQ_BAND_UNKNOWN;
	uint16_t band_str_start_idx = 0;
	uint16_t chan_str_start_idx = 0;
	uint8_t chan_idx = 0;
	uint8_t chan_start = 0;
	uint8_t chan_val = 0;
	uint16_t i = 0;
	bool valid_band = false;
	bool valid_chan = false;

	while (scan_chan_str[i] != '\0') {
		if (scan_chan_str[i] != ':') {
			i++;
			continue;
		}

		if (((i - band_str_start_idx) <= 0) ||
		    ((i - band_str_start_idx) > NRF70_MAX_BAND_STR_LEN)) {
			printf("Invalid band value %s",
				&scan_chan_str[band_str_start_idx]);
			return -EINVAL;
		}

		strncpy(band_str,
			&scan_chan_str[band_str_start_idx],
			(i - band_str_start_idx));

		band = wifi_utils_map_band_str_to_idx(band_str);

		if (band == NRF70_FREQ_BAND_UNKNOWN) {
			printf("Unsupported band value: %s", band_str);
			return -EINVAL;
		}

		i++;
		chan_str_start_idx = i;
		valid_band = true;

		while (1) {
			if ((scan_chan_str[i] != ',') &&
			    (scan_chan_str[i] != '_') &&
			    (scan_chan_str[i] != '-') &&
			    (scan_chan_str[i] != '\0')) {
				i++;
				continue;
			}

			if ((i - chan_str_start_idx) >
			    NRF70_MAX_CHAN_STR_LEN) {
				printf("Invalid chan value %s",
					&scan_chan_str[chan_str_start_idx]);
				return -EINVAL;
			}

			strncpy(chan_str,
				&scan_chan_str[chan_str_start_idx],
				(i - chan_str_start_idx));

			if (wifi_utils_validate_chan_str(chan_str)) {
				printf("Channel string validation failed");
				return -EINVAL;
			}

			chan_val = atoi(chan_str);

			memset(chan_str, 0, sizeof(chan_str));

			if (chan_start) {
				if (wifi_utils_get_all_chans_in_range(chan_start,
								      chan_val,
								      band_chan,
								      band,
								      &chan_idx)) {
					printf("Channel range invalid");
					return -EINVAL;
				}

				if (chan_idx > max_channels) {
					printf("Too many channels specified (%d)", max_channels);
					return -EINVAL;
				}
				chan_start = 0;
			} else {
				if (!wifi_utils_validate_chan(band,
							      chan_val)) {
					printf("Invalid channel %d", chan_val);
					return -EINVAL;
				}
				if (chan_idx == max_channels) {
					printf("Too many channels specified (%d)", max_channels);
					return -EINVAL;
				}

				band_chan[chan_idx].band = band;
				band_chan[chan_idx].channel = chan_val;
				chan_idx++;
			}

			valid_chan = true;

			if (scan_chan_str[i] == '_') {
				band_str_start_idx = ++i;
				break;
			} else if (scan_chan_str[i] == ',') {
				chan_str_start_idx = ++i;
			} else if (scan_chan_str[i] == '-') {
				chan_start = chan_val;
				chan_str_start_idx = ++i;
			} else if (scan_chan_str[i] == '\0') {
				break;
			}
		}
	}

	if (!valid_band) {
		printf("No valid band found");
		return -EINVAL;
	}

	if (!valid_chan) {
		printf("No valid channel found");
		return -EINVAL;
	}

	return 0;
}
