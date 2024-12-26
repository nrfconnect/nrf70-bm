/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/* @file
 * @brief NRF Wi-Fi util shell module
 */
#include <stdlib.h>
#include <nrf70_bm_core.h>

#include "host_rpu_umac_if.h"
#include "fmac_api.h"
#include "fmac_util.h"
#include "wifi_util.h"

extern struct nrf70_wifi_drv_priv_bm nrf70_bm_priv;
struct nrf70_wifi_ctx_bm *ctx = &nrf70_bm_priv.rpu_ctx_bm;

static int nrf_wifi_util_show_vers(const struct shell *shell,
				  size_t argc,
				  const char *argv[])
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	struct nrf_wifi_fmac_dev_ctx *fmac_dev_ctx = NULL;
	unsigned int fw_ver;

	fmac_dev_ctx = ctx->rpu_ctx;

	status = nrf_wifi_fmac_ver_get(fmac_dev_ctx, &fw_ver);

	if (status != NRF_WIFI_STATUS_SUCCESS) {
		shell_fprintf(shell,
		 SHELL_INFO,
		 "Failed to get firmware version\n");
		return -ENOEXEC;
	}

	shell_fprintf(shell, SHELL_INFO,
		 "Firmware version: %d.%d.%d.%d\n",
		  NRF_WIFI_UMAC_VER(fw_ver),
		  NRF_WIFI_UMAC_VER_MAJ(fw_ver),
		  NRF_WIFI_UMAC_VER_MIN(fw_ver),
		  NRF_WIFI_UMAC_VER_EXTRA(fw_ver));

	return status;
}

#ifndef CONFIG_NRF700X_RADIO_TEST
static int nrf_wifi_util_dump_rpu_stats(const struct shell *shell,
					size_t argc,
					const char *argv[])
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	struct nrf_wifi_fmac_dev_ctx *fmac_dev_ctx = NULL;
	struct rpu_op_stats stats;
	enum rpu_stats_type stats_type = RPU_STATS_TYPE_ALL;

	if (argc == 2) {
		const char *type  = argv[1];

		if (!strcmp(type, "umac")) {
			stats_type = RPU_STATS_TYPE_UMAC;
		} else if (!strcmp(type, "lmac")) {
			stats_type = RPU_STATS_TYPE_LMAC;
		} else if (!strcmp(type, "phy")) {
			stats_type = RPU_STATS_TYPE_PHY;
		} else if (!strcmp(type, "all")) {
			stats_type = RPU_STATS_TYPE_ALL;
		} else {
			shell_fprintf(shell,
				      SHELL_ERROR,
				      "Invalid stats type %s\n",
				      type);
			return -ENOEXEC;
		}
	}

	fmac_dev_ctx = ctx->rpu_ctx;

	memset(&stats, 0, sizeof(struct rpu_op_stats));
	status = nrf_wifi_fmac_stats_get(fmac_dev_ctx, 0, &stats);

	if (status != NRF_WIFI_STATUS_SUCCESS) {
		shell_fprintf(shell,
			      SHELL_ERROR,
			      "Failed to get stats\n");
		return -ENOEXEC;
	}

	if (stats_type == RPU_STATS_TYPE_UMAC || stats_type == RPU_STATS_TYPE_ALL) {
		struct rpu_umac_stats *umac = &stats.fw.umac;

		shell_fprintf(shell, SHELL_INFO,
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

		shell_fprintf(shell, SHELL_INFO,
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

		shell_fprintf(shell, SHELL_INFO,
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

			shell_fprintf(shell, SHELL_INFO,
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

		shell_fprintf(shell, SHELL_INFO,
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

		shell_fprintf(shell, SHELL_INFO,
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

#ifdef CONFIG_NRF_WIFI_RPU_RECOVERY
static int nrf_wifi_util_trigger_rpu_recovery(const struct shell *shell,
					      size_t argc,
					      const char *argv[])
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	struct nrf_wifi_fmac_dev_ctx *fmac_dev_ctx = NULL;

	if (!ctx || !ctx->rpu_ctx) {
		shell_fprintf(shell,
			      SHELL_ERROR,
			      "RPU context not initialized\n");
		return -ENOEXEC;
	}

	fmac_dev_ctx = ctx->rpu_ctx;

	status = nrf_wifi_fmac_rpu_recovery_callback(fmac_dev_ctx, NULL, 0);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		shell_fprintf(shell,
			      SHELL_ERROR,
			      "Failed to trigger RPU recovery\n");
		return -ENOEXEC;
	}

	shell_fprintf(shell,
		      SHELL_INFO,
		      "RPU recovery triggered\n");

	return 0;
}
#endif /* CONFIG_NRF_WIFI_RPU_RECOVERY */

SHELL_STATIC_SUBCMD_SET_CREATE(
	nrf_wifi_util_subcmds,
	SHELL_CMD_ARG(show_vers,
		      NULL,
		      "Display the driver and the firmware versions",
		      nrf_wifi_util_show_vers,
		      1,
		      0),
#ifndef CONFIG_NRF700X_RADIO_TEST
	SHELL_CMD_ARG(rpu_stats,
		      NULL,
		      "Display RPU stats "
		      "Parameters: umac or lmac or phy or all (default)",
		      nrf_wifi_util_dump_rpu_stats,
		      1,
		      1),
#endif /* CONFIG_NRF700X_RADIO_TEST */
#ifdef CONFIG_NRF_WIFI_RPU_RECOVERY
	SHELL_CMD_ARG(rpu_recovery_test,
		      NULL,
		      "Trigger RPU recovery",
		      nrf_wifi_util_trigger_rpu_recovery,
		      1,
		      0),
#endif /* CONFIG_NRF_WIFI_RPU_RECOVERY */
	SHELL_SUBCMD_SET_END);


SHELL_CMD_REGISTER(wifi_util,
		   &nrf_wifi_util_subcmds,
		   "nRF Wi-Fi utility shell commands",
		   NULL);
