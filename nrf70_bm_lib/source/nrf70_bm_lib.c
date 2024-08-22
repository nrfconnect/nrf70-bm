/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief nRF70 Bare Metal library.
 */

#include "nrf70_bm_lib.h"
#include "nrf70_bm_core.h"

#include "util.h"

#ifndef CONFIG_NRF700X_RADIO_TEST
/* Overlay struct to avoid dynamic memory allocation */
typedef struct  __attribute__((packed)) scan_info_overlay {
	struct nrf_wifi_umac_scan_info scan_info;
	unsigned int center_frequency[NRF70_SCAN_CHAN_MAX_MANUAL];
} scan_info_overlay_t;

const char *nrf70_security_txt(enum nrf70_security_type security)
{
	switch (security) {
	case NRF70_SECURITY_TYPE_NONE:
		return "OPEN";
	case NRF70_SECURITY_TYPE_WEP:
		return "WEP";
	case NRF70_SECURITY_TYPE_WPA_PSK:
		return "WPA-PSK";
	case NRF70_SECURITY_TYPE_PSK:
		return "WPA2-PSK";
	case NRF70_SECURITY_TYPE_PSK_SHA256:
		return "WPA2-PSK-SHA256";
	case NRF70_SECURITY_TYPE_SAE:
		return "WPA3-SAE";
	case NRF70_SECURITY_TYPE_WAPI:
		return "WAPI";
	case NRF70_SECURITY_TYPE_EAP:
		return "EAP";
	case NRF70_SECURITY_TYPE_UNKNOWN:
	default:
		return "UNKNOWN";
	}
}

const char *nrf70_mfp_txt(enum nrf70_mfp_options mfp)
{
	switch (mfp) {
	case NRF70_MFP_DISABLE:
		return "Disable";
	case NRF70_MFP_OPTIONAL:
		return "Optional";
	case NRF70_MFP_REQUIRED:
		return "Required";
	case NRF70_MFP_UNKNOWN:
	default:
		return "UNKNOWN";
	}
}

const char *nrf70_band_txt(enum nrf70_frequency_bands band)
{
	switch (band) {
	case NRF70_FREQ_BAND_2_4_GHZ:
		return "2.4GHz";
	case NRF70_FREQ_BAND_5_GHZ:
		return "5GHz";
	case NRF70_FREQ_BAND_6_GHZ:
		return "6GHz";
	case NRF70_FREQ_BAND_UNKNOWN:
	default:
		return "UNKNOWN";
	}
}

static enum nrf_wifi_band nrf_wifi_map_band_to_rpu(enum nrf70_frequency_bands band)
{
	switch (band) {
	case NRF70_FREQ_BAND_2_4_GHZ:
		return NRF_WIFI_BAND_2GHZ;
	case NRF70_FREQ_BAND_5_GHZ:
		return NRF_WIFI_BAND_5GHZ;
	default:
		return NRF_WIFI_BAND_INVALID;
	}
}
#endif /* CONFIG_NRF700X_RADIO_TEST */

int nrf70_init(void)
{
	int ret;

	// Initialize the WiFi module
	ret = nrf70_fmac_init();
	if (ret) {
		NRF70_LOG_ERR("Failed to initialize FMAC module");
		goto err;
	}
#ifndef CONFIG_NRF700X_RADIO_TEST
	ret = nrf70_fmac_add_vif_sta();
	if (ret) {
		NRF70_LOG_ERR("Failed to add STA VIF");
		goto deinit;
	}
#endif /* CONFIG_NRF700X_RADIO_TEST */
	return 0;
#ifndef CONFIG_NRF700X_RADIO_TEST
deinit:
	nrf70_fmac_deinit();
#endif /* CONFIG_NRF700X_RADIO_TEST */
err:
	return ret;
}

#ifndef CONFIG_NRF700X_RADIO_TEST
int nrf70_scan_start(struct nrf70_scan_params *params,
					 nrf70_scan_result_cb_t cb)
{
	// Start scanning for WiFi networks
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	scan_info_overlay_t scan_info_overlay = { 0 };
	struct nrf_wifi_umac_scan_info *scan_info = &scan_info_overlay.scan_info;
	enum nrf_wifi_band band = NRF_WIFI_BAND_INVALID;
	uint8_t skip_local_admin_mac = 0;
	uint8_t band_flags = 0xFF;
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t k = 0;
	uint16_t num_scan_channels = 0;
	int ret = -1;
	void *rpu_ctx = nrf70_bm_priv.rpu_ctx_bm.rpu_ctx;
	struct nrf70_wifi_vif_bm *vif = &nrf70_bm_priv.rpu_ctx_bm.vifs[0];
	struct nrf_wifi_fmac_priv *fmac_priv = nrf70_bm_priv.fmac_priv;

	if (!params) {
		NRF70_LOG_DBG("No scan parameters provided, using default values");
	}

	if (!cb) {
		NRF70_LOG_ERR("Invalid scan result callback");
		goto err;
	}

	if (!rpu_ctx) {
		NRF70_LOG_ERR("Invalid RPU context");
		goto err;
	}

	if (!fmac_priv) {
		NRF70_LOG_ERR("Invalid FMAC private data");
		goto err;
	}

	if (vif->vif_idx >= MAX_NUM_VIFS) {
		NRF70_LOG_ERR("Invalid VIF index: %d", vif->vif_idx);
		goto err;
	}

	if (vif->op_state != NRF_WIFI_FMAC_IF_OP_STATE_UP) {
		NRF70_LOG_ERR("VIF not in UP state");
		goto err;
	}

	if (vif->scan_in_progress) {
		NRF70_LOG_ERR("Scan already in progress");
		goto err;
	}

	// Set the scan result callback
	vif->scan_result_cb = (void *)cb;

	// Set the scan parameters
	if (params) {
		band_flags &= (~(1 << NRF70_FREQ_BAND_2_4_GHZ));

#ifndef CONFIG_NRF70_2_4G_ONLY
		band_flags &= (~(1 << NRF70_FREQ_BAND_5_GHZ));
#endif /* CONFIG_NRF70_2_4G_ONLY */

		if (params->bands & band_flags) {
			NRF70_LOG_ERR("%s: Unsupported band(s) (0x%X)", __func__, params->bands);
			goto err;
		}

		for (j = 0; j < CONFIG_NRF70_SCAN_CHAN_MAX_MANUAL; j++) {
			if (!params->band_chan[j].channel) {
				break;
			}

			num_scan_channels++;
		}
	}

	memset(scan_info, 0, sizeof(*scan_info));

#ifdef CONFIG_NRF70_NRF700X_SKIP_LOCAL_ADMIN_MAC
	skip_local_admin_mac = 1;
#endif /* CONFIG_NRF70_NRF700X_SKIP_LOCAL_ADMIN_MAC */

	scan_info->scan_params.skip_local_admin_macs = skip_local_admin_mac;

	scan_info->scan_reason = SCAN_DISPLAY;

	if (params) {
		if (params->scan_type == NRF70_SCAN_TYPE_PASSIVE) {
			scan_info->scan_params.passive_scan = 1;
		}

		scan_info->scan_params.bands = params->bands;

		if (params->dwell_time_active < 0) {
			NRF70_LOG_ERR("%s: Invalid dwell_time_active %d", __func__,
				params->dwell_time_active);
			goto err;
		} else {
			scan_info->scan_params.dwell_time_active = params->dwell_time_active;
		}

		if (params->dwell_time_passive < 0) {
			NRF70_LOG_ERR("%s: Invalid dwell_time_passive %d", __func__,
				params->dwell_time_passive);
			goto err;
		} else {
			scan_info->scan_params.dwell_time_passive = params->dwell_time_passive;
		}

		if ((params->max_bss_cnt < 0) ||
		    (params->max_bss_cnt > NRF70_SCAN_MAX_BSS_CNT)) {
			NRF70_LOG_ERR("%s: Invalid max_bss_cnt %d", __func__,
				params->max_bss_cnt);
			goto err;
		} else {
			vif->max_bss_cnt = params->max_bss_cnt;
		}

		for (i = 0; i < NRF_WIFI_SCAN_MAX_NUM_SSIDS; i++) {
			if (!(params->ssids[i]) || !strlen(params->ssids[i])) {
				break;
			}

			memcpy(scan_info->scan_params.scan_ssids[i].nrf_wifi_ssid,
			       params->ssids[i],
			       sizeof(scan_info->scan_params.scan_ssids[i].nrf_wifi_ssid));

			scan_info->scan_params.scan_ssids[i].nrf_wifi_ssid_len =
				strlen(scan_info->scan_params.scan_ssids[i].nrf_wifi_ssid);

			scan_info->scan_params.num_scan_ssids++;
		}

		for (i = 0; i < CONFIG_NRF70_SCAN_CHAN_MAX_MANUAL; i++) {
			if (!params->band_chan[i].channel) {
				break;
			}

			band = nrf_wifi_map_band_to_rpu(params->band_chan[i].band);

			if (band == NRF_WIFI_BAND_INVALID) {
				NRF70_LOG_ERR("%s: Unsupported band %d", __func__,
					params->band_chan[i].band);
				goto err;
			}

			scan_info_overlay.center_frequency[k++] = nrf_wifi_utils_chan_to_freq(
				fmac_priv->opriv, band, params->band_chan[i].channel);

			if (scan_info_overlay.center_frequency[k - 1] == -1) {
				NRF70_LOG_ERR("%s: Invalid channel %d", __func__,
					 params->band_chan[i].channel);
				goto err;
			}
		}

		scan_info->scan_params.num_scan_channels = k;
	}

	vif->scan_res_cnt = 0;

	status = nrf_wifi_fmac_scan(rpu_ctx,
								vif->vif_idx,
								(struct nrf_wifi_umac_scan_info *)&scan_info_overlay);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: nrf_wifi_fmac_scan failed", __func__);
		goto err;
	}

	NRF70_LOG_DBG("Scan started");
	
	return 0;
err:
	return ret;
}

bool nrf70_scan_done(void)
{
	struct nrf70_wifi_vif_bm *vif = &nrf70_bm_priv.rpu_ctx_bm.vifs[0];

	return vif->scan_done;
}
#endif /* CONFIG_NRF700X_RADIO_TEST */

int nrf70_deinit(void)
{
	int

#ifndef CONFIG_NRF700X_RADIO_TEST
	ret = nrf70_fmac_del_vif_sta();
	if (ret) {
		NRF70_LOG_ERR("Failed to delete STA VIF");
		goto err;
	}
#endif /* CONFIG_NRF700X_RADIO_TEST */

	// Clean up the WiFi module
	ret = nrf70_fmac_deinit();
	if (ret) {
		NRF70_LOG_ERR("Failed to deinitialize FMAC module");
		goto err;
	}

	return 0;
err:
	return ret;
}

#ifndef CONFIG_NRF700X_RADIO_TEST
int nrf_wifi_util_dump_rpu_stats(const char *type)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	struct rpu_op_stats stats;
	enum rpu_stats_type stats_type = RPU_STATS_TYPE_ALL;
	void *rpu_ctx = nrf70_bm_priv.rpu_ctx_bm.rpu_ctx;

	if (!strcmp(type, "umac")) {
		stats_type = RPU_STATS_TYPE_UMAC;
	} else if (!strcmp(type, "lmac")) {
		stats_type = RPU_STATS_TYPE_LMAC;
	} else if (!strcmp(type, "phy")) {
		stats_type = RPU_STATS_TYPE_PHY;
	} else if (!strcmp(type, "all")) {
		stats_type = RPU_STATS_TYPE_ALL;
	} else {
		NRF70_LOG_ERR("Invalid stats type\n");
		return -1;
	}

	memset(&stats, 0, sizeof(struct rpu_op_stats));
	status = nrf_wifi_fmac_stats_get(rpu_ctx, 0, &stats);

	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("Failed to get stats\n");
		return -1;
	}

	if (stats_type == RPU_STATS_TYPE_UMAC || stats_type == RPU_STATS_TYPE_ALL) {
		struct rpu_umac_stats *umac = &stats.fw.umac;

		NRF70_LOG_INF(
				  "UMAC TX debug stats:\n"
				  "======================\n"
				  "tx_cmd: %u\n"
				  "tx_non_coalesce_pkts_rcvd_from_host: %u\n"
				  "tx_coalesce_pkts_rcvd_from_host: %u\n"
				  "tx_max_coalesce_pkts_rcvd_from_host: %u\n"
				  "tx_cmds_max_used: %u\n"
				  "tx_cmds_currently_in_use: %u\n"
				  "tx_done_events_send_to_host: %u\n"
				  "tx_done_success_pkts_to_host: %u\n"
				  "tx_done_failure_pkts_to_host: %u\n"
				  "tx_cmds_with_crypto_pkts_rcvd_from_host: %u\n"
				  "tx_cmds_with_non_crypto_pkts_rcvd_from_host: %u\n"
				  "tx_cmds_with_broadcast_pkts_rcvd_from_host: %u\n"
				  "tx_cmds_with_multicast_pkts_rcvd_from_host: %u\n"
				  "tx_cmds_with_unicast_pkts_rcvd_from_host: %u\n"
				  "xmit: %u\n"
				  "send_addba_req: %u\n"
				  "addba_resp: %u\n"
				  "softmac_tx: %u\n"
				  "internal_pkts: %u\n"
				  "external_pkts: %u\n"
				  "tx_cmds_to_lmac: %u\n"
				  "tx_dones_from_lmac: %u\n"
				  "total_cmds_to_lmac: %u\n"
				  "tx_packet_data_count: %u\n"
				  "tx_packet_mgmt_count: %u\n"
				  "tx_packet_beacon_count: %u\n"
				  "tx_packet_probe_req_count: %u\n"
				  "tx_packet_auth_count: %u\n"
				  "tx_packet_deauth_count: %u\n"
				  "tx_packet_assoc_req_count: %u\n"
				  "tx_packet_disassoc_count: %u\n"
				  "tx_packet_action_count: %u\n"
				  "tx_packet_other_mgmt_count: %u\n"
				  "tx_packet_non_mgmt_data_count: %u\n\n",
				  umac->tx_dbg_params.tx_cmd,
				  umac->tx_dbg_params.tx_non_coalesce_pkts_rcvd_from_host,
				  umac->tx_dbg_params.tx_coalesce_pkts_rcvd_from_host,
				  umac->tx_dbg_params.tx_max_coalesce_pkts_rcvd_from_host,
				  umac->tx_dbg_params.tx_cmds_max_used,
				  umac->tx_dbg_params.tx_cmds_currently_in_use,
				  umac->tx_dbg_params.tx_done_events_send_to_host,
				  umac->tx_dbg_params.tx_done_success_pkts_to_host,
				  umac->tx_dbg_params.tx_done_failure_pkts_to_host,
				  umac->tx_dbg_params.tx_cmds_with_crypto_pkts_rcvd_from_host,
				  umac->tx_dbg_params.tx_cmds_with_non_crypto_pkts_rcvd_from_host,
				  umac->tx_dbg_params.tx_cmds_with_broadcast_pkts_rcvd_from_host,
				  umac->tx_dbg_params.tx_cmds_with_multicast_pkts_rcvd_from_host,
				  umac->tx_dbg_params.tx_cmds_with_unicast_pkts_rcvd_from_host,
				  umac->tx_dbg_params.xmit,
				  umac->tx_dbg_params.send_addba_req,
				  umac->tx_dbg_params.addba_resp,
				  umac->tx_dbg_params.softmac_tx,
				  umac->tx_dbg_params.internal_pkts,
				  umac->tx_dbg_params.external_pkts,
				  umac->tx_dbg_params.tx_cmds_to_lmac,
				  umac->tx_dbg_params.tx_dones_from_lmac,
				  umac->tx_dbg_params.total_cmds_to_lmac,
				  umac->tx_dbg_params.tx_packet_data_count,
				  umac->tx_dbg_params.tx_packet_mgmt_count,
				  umac->tx_dbg_params.tx_packet_beacon_count,
				  umac->tx_dbg_params.tx_packet_probe_req_count,
				  umac->tx_dbg_params.tx_packet_auth_count,
				  umac->tx_dbg_params.tx_packet_deauth_count,
				  umac->tx_dbg_params.tx_packet_assoc_req_count,
				  umac->tx_dbg_params.tx_packet_disassoc_count,
				  umac->tx_dbg_params.tx_packet_action_count,
				  umac->tx_dbg_params.tx_packet_other_mgmt_count,
				  umac->tx_dbg_params.tx_packet_non_mgmt_data_count);

		NRF70_LOG_INF(
				  "UMAC RX debug stats\n"
				  "======================\n"
				  "lmac_events: %u\n"
				  "rx_events: %u\n"
				  "rx_coalesce_events: %u\n"
				  "total_rx_pkts_from_lmac: %u\n"
				  "max_refill_gap: %u\n"
				  "current_refill_gap: %u\n"
				  "out_of_order_mpdus: %u\n"
				  "reorder_free_mpdus: %u\n"
				  "umac_consumed_pkts: %u\n"
				  "host_consumed_pkts: %u\n"
				  "rx_mbox_post: %u\n"
				  "rx_mbox_receive: %u\n"
				  "reordering_ampdu: %u\n"
				  "timer_mbox_post: %u\n"
				  "timer_mbox_rcv: %u\n"
				  "work_mbox_post: %u\n"
				  "work_mbox_rcv: %u\n"
				  "tasklet_mbox_post: %u\n"
				  "tasklet_mbox_rcv: %u\n"
				  "userspace_offload_frames: %u\n"
				  "alloc_buf_fail: %u\n"
				  "rx_packet_total_count: %u\n"
				  "rx_packet_data_count: %u\n"
				  "rx_packet_qos_data_count: %u\n"
				  "rx_packet_protected_data_count: %u\n"
				  "rx_packet_mgmt_count: %u\n"
				  "rx_packet_beacon_count: %u\n"
				  "rx_packet_probe_resp_count: %u\n"
				  "rx_packet_auth_count: %u\n"
				  "rx_packet_deauth_count: %u\n"
				  "rx_packet_assoc_resp_count: %u\n"
				  "rx_packet_disassoc_count: %u\n"
				  "rx_packet_action_count: %u\n"
				  "rx_packet_probe_req_count: %u\n"
				  "rx_packet_other_mgmt_count: %u\n"
				  "max_coalesce_pkts: %d\n"
				  "null_skb_pointer_from_lmac: %u\n"
				  "unexpected_mgmt_pkt: %u\n\n",
				  umac->rx_dbg_params.lmac_events,
				  umac->rx_dbg_params.rx_events,
				  umac->rx_dbg_params.rx_coalesce_events,
				  umac->rx_dbg_params.total_rx_pkts_from_lmac,
				  umac->rx_dbg_params.max_refill_gap,
				  umac->rx_dbg_params.current_refill_gap,
				  umac->rx_dbg_params.out_of_order_mpdus,
				  umac->rx_dbg_params.reorder_free_mpdus,
				  umac->rx_dbg_params.umac_consumed_pkts,
				  umac->rx_dbg_params.host_consumed_pkts,
				  umac->rx_dbg_params.rx_mbox_post,
				  umac->rx_dbg_params.rx_mbox_receive,
				  umac->rx_dbg_params.reordering_ampdu,
				  umac->rx_dbg_params.timer_mbox_post,
				  umac->rx_dbg_params.timer_mbox_rcv,
				  umac->rx_dbg_params.work_mbox_post,
				  umac->rx_dbg_params.work_mbox_rcv,
				  umac->rx_dbg_params.tasklet_mbox_post,
				  umac->rx_dbg_params.tasklet_mbox_rcv,
				  umac->rx_dbg_params.userspace_offload_frames,
				  umac->rx_dbg_params.alloc_buf_fail,
				  umac->rx_dbg_params.rx_packet_total_count,
				  umac->rx_dbg_params.rx_packet_data_count,
				  umac->rx_dbg_params.rx_packet_qos_data_count,
				  umac->rx_dbg_params.rx_packet_protected_data_count,
				  umac->rx_dbg_params.rx_packet_mgmt_count,
				  umac->rx_dbg_params.rx_packet_beacon_count,
				  umac->rx_dbg_params.rx_packet_probe_resp_count,
				  umac->rx_dbg_params.rx_packet_auth_count,
				  umac->rx_dbg_params.rx_packet_deauth_count,
				  umac->rx_dbg_params.rx_packet_assoc_resp_count,
				  umac->rx_dbg_params.rx_packet_disassoc_count,
				  umac->rx_dbg_params.rx_packet_action_count,
				  umac->rx_dbg_params.rx_packet_probe_req_count,
				  umac->rx_dbg_params.rx_packet_other_mgmt_count,
				  umac->rx_dbg_params.max_coalesce_pkts,
				  umac->rx_dbg_params.null_skb_pointer_from_lmac,
				  umac->rx_dbg_params.unexpected_mgmt_pkt);

		NRF70_LOG_INF(
				  "UMAC control path stats\n"
				  "======================\n"
				  "cmd_init: %u\n"
				  "event_init_done: %u\n"
				  "cmd_rf_test: %u\n"
				  "cmd_connect: %u\n"
				  "cmd_get_stats: %u\n"
				  "event_ps_state: %u\n"
				  "cmd_set_reg: %u\n"
				  "cmd_get_reg: %u\n"
				  "cmd_req_set_reg: %u\n"
				  "cmd_trigger_scan: %u\n"
				  "event_scan_done: %u\n"
				  "cmd_get_scan: %u\n"
				  "umac_scan_req: %u\n"
				  "umac_scan_complete: %u\n"
				  "umac_scan_busy: %u\n"
				  "cmd_auth: %u\n"
				  "cmd_assoc: %u\n"
				  "cmd_deauth: %u\n"
				  "cmd_register_frame: %u\n"
				  "cmd_frame: %u\n"
				  "cmd_del_key: %u\n"
				  "cmd_new_key: %u\n"
				  "cmd_set_key: %u\n"
				  "cmd_get_key: %u\n"
				  "event_beacon_hint: %u\n"
				  "event_reg_change: %u\n"
				  "event_wiphy_reg_change: %u\n"
				  "cmd_set_station: %u\n"
				  "cmd_new_station: %u\n"
				  "cmd_del_station: %u\n"
				  "cmd_new_interface: %u\n"
				  "cmd_set_interface: %u\n"
				  "cmd_get_interface: %u\n"
				  "cmd_set_ifflags: %u\n"
				  "cmd_set_ifflags_done: %u\n"
				  "cmd_set_bss: %u\n"
				  "cmd_set_wiphy: %u\n"
				  "cmd_start_ap: %u\n"
				  "LMAC_CMD_PS: %u\n"
				  "CURR_STATE: %u\n\n",
				  umac->cmd_evnt_dbg_params.cmd_init,
				  umac->cmd_evnt_dbg_params.event_init_done,
				  umac->cmd_evnt_dbg_params.cmd_rf_test,
				  umac->cmd_evnt_dbg_params.cmd_connect,
				  umac->cmd_evnt_dbg_params.cmd_get_stats,
				  umac->cmd_evnt_dbg_params.event_ps_state,
				  umac->cmd_evnt_dbg_params.cmd_set_reg,
				  umac->cmd_evnt_dbg_params.cmd_get_reg,
				  umac->cmd_evnt_dbg_params.cmd_req_set_reg,
				  umac->cmd_evnt_dbg_params.cmd_trigger_scan,
				  umac->cmd_evnt_dbg_params.event_scan_done,
				  umac->cmd_evnt_dbg_params.cmd_get_scan,
				  umac->cmd_evnt_dbg_params.umac_scan_req,
				  umac->cmd_evnt_dbg_params.umac_scan_complete,
				  umac->cmd_evnt_dbg_params.umac_scan_busy,
				  umac->cmd_evnt_dbg_params.cmd_auth,
				  umac->cmd_evnt_dbg_params.cmd_assoc,
				  umac->cmd_evnt_dbg_params.cmd_deauth,
				  umac->cmd_evnt_dbg_params.cmd_register_frame,
				  umac->cmd_evnt_dbg_params.cmd_frame,
				  umac->cmd_evnt_dbg_params.cmd_del_key,
				  umac->cmd_evnt_dbg_params.cmd_new_key,
				  umac->cmd_evnt_dbg_params.cmd_set_key,
				  umac->cmd_evnt_dbg_params.cmd_get_key,
				  umac->cmd_evnt_dbg_params.event_beacon_hint,
				  umac->cmd_evnt_dbg_params.event_reg_change,
				  umac->cmd_evnt_dbg_params.event_wiphy_reg_change,
				  umac->cmd_evnt_dbg_params.cmd_set_station,
				  umac->cmd_evnt_dbg_params.cmd_new_station,
				  umac->cmd_evnt_dbg_params.cmd_del_station,
				  umac->cmd_evnt_dbg_params.cmd_new_interface,
				  umac->cmd_evnt_dbg_params.cmd_set_interface,
				  umac->cmd_evnt_dbg_params.cmd_get_interface,
				  umac->cmd_evnt_dbg_params.cmd_set_ifflags,
				  umac->cmd_evnt_dbg_params.cmd_set_ifflags_done,
				  umac->cmd_evnt_dbg_params.cmd_set_bss,
				  umac->cmd_evnt_dbg_params.cmd_set_wiphy,
				  umac->cmd_evnt_dbg_params.cmd_start_ap,
				  umac->cmd_evnt_dbg_params.LMAC_CMD_PS,
				  umac->cmd_evnt_dbg_params.CURR_STATE);

			NRF70_LOG_INF(
				  "UMAC interface stats\n"
				  "======================\n"
				  "tx_unicast_pkt_count: %u\n"
				  "tx_multicast_pkt_count: %u\n"
				  "tx_broadcast_pkt_count: %u\n"
				  "tx_bytes: %u\n"
				  "rx_unicast_pkt_count: %u\n"
				  "rx_multicast_pkt_count: %u\n"
				  "rx_broadcast_pkt_count: %u\n"
				  "rx_beacon_success_count: %u\n"
				  "rx_beacon_miss_count: %u\n"
				  "rx_bytes: %u\n"
				  "rx_checksum_error_count: %u\n\n"
				  "replay_attack_drop_cnt: %u\n\n",
				  umac->interface_data_stats.tx_unicast_pkt_count,
				  umac->interface_data_stats.tx_multicast_pkt_count,
				  umac->interface_data_stats.tx_broadcast_pkt_count,
				  umac->interface_data_stats.tx_bytes,
				  umac->interface_data_stats.rx_unicast_pkt_count,
				  umac->interface_data_stats.rx_multicast_pkt_count,
				  umac->interface_data_stats.rx_broadcast_pkt_count,
				  umac->interface_data_stats.rx_beacon_success_count,
				  umac->interface_data_stats.rx_beacon_miss_count,
				  umac->interface_data_stats.rx_bytes,
				  umac->interface_data_stats.rx_checksum_error_count,
				  umac->interface_data_stats.replay_attack_drop_cnt);
	}

	if (stats_type == RPU_STATS_TYPE_LMAC || stats_type == RPU_STATS_TYPE_ALL) {
		struct rpu_lmac_stats *lmac = &stats.fw.lmac;

		NRF70_LOG_INF(
			      "LMAC stats\n"
				  "======================\n"
				  "reset_cmd_cnt: %u\n"
				  "reset_complete_event_cnt: %u\n"
				  "unable_gen_event: %u\n"
				  "ch_prog_cmd_cnt: %u\n"
				  "channel_prog_done: %u\n"
				  "tx_pkt_cnt: %u\n"
				  "tx_pkt_done_cnt: %u\n"
				  "scan_pkt_cnt: %u\n"
				  "internal_pkt_cnt: %u\n"
				  "internal_pkt_done_cnt: %u\n"
				  "ack_resp_cnt: %u\n"
				  "tx_timeout: %u\n"
				  "deagg_isr: %u\n"
				  "deagg_inptr_desc_empty: %u\n"
				  "deagg_circular_buffer_full: %u\n"
				  "lmac_rxisr_cnt: %u\n"
				  "rx_decryptcnt: %u\n"
				  "process_decrypt_fail: %u\n"
				  "prepa_rx_event_fail: %u\n"
				  "rx_core_pool_full_cnt: %u\n"
				  "rx_mpdu_crc_success_cnt: %u\n"
				  "rx_mpdu_crc_fail_cnt: %u\n"
				  "rx_ofdm_crc_success_cnt: %u\n"
				  "rx_ofdm_crc_fail_cnt: %u\n"
				  "rxDSSSCrcSuccessCnt: %u\n"
				  "rxDSSSCrcFailCnt: %u\n"
				  "rx_crypto_start_cnt: %u\n"
				  "rx_crypto_done_cnt: %u\n"
				  "rx_event_buf_full: %u\n"
				  "rx_extram_buf_full: %u\n"
				  "scan_req: %u\n"
				  "scan_complete: %u\n"
				  "scan_abort_req: %u\n"
				  "scan_abort_complete: %u\n"
				  "internal_buf_pool_null: %u\n"
				  "rpu_hw_lockup_count: %u\n"
				  "rpu_hw_lockup_recovery_done: %u\n\n",
				  lmac->reset_cmd_cnt,
				  lmac->reset_complete_event_cnt,
				  lmac->unable_gen_event,
				  lmac->ch_prog_cmd_cnt,
				  lmac->channel_prog_done,
				  lmac->tx_pkt_cnt,
				  lmac->tx_pkt_done_cnt,
				  lmac->scan_pkt_cnt,
				  lmac->internal_pkt_cnt,
				  lmac->internal_pkt_done_cnt,
				  lmac->ack_resp_cnt,
				  lmac->tx_timeout,
				  lmac->deagg_isr,
				  lmac->deagg_inptr_desc_empty,
				  lmac->deagg_circular_buffer_full,
				  lmac->lmac_rxisr_cnt,
				  lmac->rx_decryptcnt,
				  lmac->process_decrypt_fail,
				  lmac->prepa_rx_event_fail,
				  lmac->rx_core_pool_full_cnt,
				  lmac->rx_mpdu_crc_success_cnt,
				  lmac->rx_mpdu_crc_fail_cnt,
				  lmac->rx_ofdm_crc_success_cnt,
				  lmac->rx_ofdm_crc_fail_cnt,
				  lmac->rxDSSSCrcSuccessCnt,
				  lmac->rxDSSSCrcFailCnt,
				  lmac->rx_crypto_start_cnt,
				  lmac->rx_crypto_done_cnt,
				  lmac->rx_event_buf_full,
				  lmac->rx_extram_buf_full,
				  lmac->scan_req,
				  lmac->scan_complete,
				  lmac->scan_abort_req,
				  lmac->scan_abort_complete,
				  lmac->internal_buf_pool_null,
				  lmac->rpu_hw_lockup_count,
				  lmac->rpu_hw_lockup_recovery_done);
	}

	if (stats_type == RPU_STATS_TYPE_PHY || stats_type == RPU_STATS_TYPE_ALL) {
		struct rpu_phy_stats *phy = &stats.fw.phy;

		NRF70_LOG_INF(
			      "PHY stats\n"
				  "======================\n"
				  "rssi_avg: %d\n"
				  "pdout_val: %u\n"
				  "ofdm_crc32_pass_cnt: %u\n"
				  "ofdm_crc32_fail_cnt: %u\n"
				  "dsss_crc32_pass_cnt: %u\n"
				  "dsss_crc32_fail_cnt: %u\n\n",
				  phy->rssi_avg,
				  phy->pdout_val,
				  phy->ofdm_crc32_pass_cnt,
				  phy->ofdm_crc32_fail_cnt,
				  phy->dsss_crc32_pass_cnt,
				  phy->dsss_crc32_fail_cnt);
	}

	return 0;
}
#endif /* CONFIG_NRF700X_RADIO_TEST */
