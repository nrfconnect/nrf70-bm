/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief nRF70 Bare Metal initialization.
 */
#include "nrf70_bm_core.h"
#include "nrf70_bm_lib.h"

#include "util.h"
#include "fmac_api.h"
#include "fmac_util.h"

struct nrf70_wifi_drv_priv_bm nrf70_bm_priv;

/* INCBIN macro Taken from https://gist.github.com/mmozeiko/ed9655cf50341553d282 */
#define STR2(x) #x
#define STR(x) STR2(x)

#ifdef __APPLE__
#define USTR(x) "_" STR(x)
#else
#define USTR(x) STR(x)
#endif

#ifdef _WIN32
#define INCBIN_SECTION ".rdata, \"dr\""
#elif defined __APPLE__
#define INCBIN_SECTION "__TEXT,__const"
#else
#define INCBIN_SECTION ".rodata.*"
#endif

/* this aligns start address to 16 and terminates byte array with explicit 0
 * which is not really needed, feel free to change it to whatever you want/need
 */
#define INCBIN(prefix, name, file) \
	__asm__(".section " INCBIN_SECTION "\n" \
			".global " USTR(prefix) "_" STR(name) "_start\n" \
			".balign 16\n" \
			USTR(prefix) "_" STR(name) "_start:\n" \
			".incbin \"" file "\"\n" \
			\
			".global " STR(prefix) "_" STR(name) "_end\n" \
			".balign 1\n" \
			USTR(prefix) "_" STR(name) "_end:\n" \
			".byte 0\n" \
	); \
	extern __aligned(16)    const char prefix ## _ ## name ## _start[]; \
	extern                  const char prefix ## _ ## name ## _end[];

INCBIN(_bin, nrf70_fw, STR(CONFIG_NRF_WIFI_FW_BIN));


void nrf70_mac_txt(const unsigned char *mac, char *mac_str, size_t size)
{
	if (size < 18) {
		// Handle error: buffer too small
		return;
	}

	snprintf(mac_str, size, "%02X:%02X:%02X:%02X:%02X:%02X",
			 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static enum nrf_wifi_status nrf_wifi_fw_load(void *rpu_ctx)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	struct nrf_wifi_fmac_fw_info fw_info = { 0 };
	uint8_t *fw_start;
	uint8_t *fw_end;

	fw_start = (uint8_t *)_bin_nrf70_fw_start;
	fw_end = (uint8_t *)_bin_nrf70_fw_end;

	status = nrf_wifi_fmac_fw_parse(rpu_ctx, fw_start, fw_end - fw_start,
					&fw_info);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: nrf_wifi_fmac_fw_parse failed", __func__);
		return status;
	}
	/* Load the FW patches to the RPU */
	status = nrf_wifi_fmac_fw_load(rpu_ctx, &fw_info);

	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: nrf_wifi_fmac_fw_load failed", __func__);
	}

	return status;
}

#ifndef CONFIG_NRF70_RADIO_TEST
static void reg_change_callbk_fn(void *vif_ctx,
			  struct nrf_wifi_event_regulatory_change *reg_change_event,
			  unsigned int event_len)
{
	NRF70_LOG_DBG("Regulatory change event received");
}

static void nrf_wifi_event_proc_scan_start_zep(void *vif_ctx,
				struct nrf_wifi_umac_event_trigger_scan *scan_start_event,
				unsigned int event_len)
{
	NRF70_LOG_DBG("Scan started event received");
}

static void nrf_wifi_event_proc_scan_done_zep(void *vif_ctx,
				struct nrf_wifi_umac_event_trigger_scan *scan_done_event,
				unsigned int event_len)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	struct nrf70_wifi_vif_bm *vif = vif_ctx;
	void *rpu_ctx = nrf70_bm_priv.rpu_ctx_bm.rpu_ctx;
	NRF70_LOG_DBG("Scan done event received");

	status = nrf_wifi_fmac_scan_res_get(rpu_ctx,
					    vif->vif_idx,
					    SCAN_DISPLAY);

	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: failed", __func__);
		goto err;
	}
err:
	return;
}

static inline enum nrf70_mfp_options drv_to_bm_mfp(unsigned char mfp_flag)
{
	if (!mfp_flag)
		return NRF70_MFP_DISABLE;
	if (mfp_flag & NRF_WIFI_MFP_REQUIRED)
		return NRF70_MFP_REQUIRED;
	if (mfp_flag & NRF_WIFI_MFP_CAPABLE)
		return NRF70_MFP_OPTIONAL;

	return NRF70_MFP_UNKNOWN;
}
static inline enum nrf70_security_type drv_to_bm(int drv_security_type)
{
	switch (drv_security_type) {
	case NRF_WIFI_OPEN:
		return NRF70_SECURITY_TYPE_NONE;
	case NRF_WIFI_WEP:
		return NRF70_SECURITY_TYPE_WEP;
	case NRF_WIFI_WPA:
		return NRF70_SECURITY_TYPE_WPA_PSK;
	case NRF_WIFI_WPA2:
		return NRF70_SECURITY_TYPE_PSK;
	case NRF_WIFI_WPA2_256:
		return NRF70_SECURITY_TYPE_PSK_SHA256;
	case NRF_WIFI_WPA3:
		return NRF70_SECURITY_TYPE_SAE;
	case NRF_WIFI_WAPI:
		return NRF70_SECURITY_TYPE_WAPI;
	case NRF_WIFI_EAP:
		return NRF70_SECURITY_TYPE_EAP;
	default:
		return NRF70_SECURITY_TYPE_UNKNOWN;
	}
}

static void nrf_wifi_event_proc_disp_scan_res_zep(void *vif_ctx,
				struct nrf_wifi_umac_event_new_scan_display_results *scan_res,
				unsigned int event_len,
				bool more_res)
{
	struct nrf70_wifi_vif_bm *vif = vif_ctx;
	uint16_t max_bss_cnt = 0;
	struct umac_display_results *r = NULL;
	struct nrf70_scan_result res;
	unsigned int i;


	NRF70_LOG_DBG("Scan result event received");
	max_bss_cnt = vif->max_bss_cnt ?
		vif->max_bss_cnt : CONFIG_NRF_WIFI_SCAN_MAX_BSS_CNT;

	for (i = 0; i < scan_res->event_bss_count; i++) {
		/* Limit the scan results to the configured maximum */
		if ((max_bss_cnt > 0) &&
		    (vif->scan_res_cnt >= max_bss_cnt)) {
			break;
		}

		memset(&res, 0x0, sizeof(res));

		r = &scan_res->display_results[i];

		res.ssid_len = MIN(sizeof(res.ssid), r->ssid.nrf_wifi_ssid_len);

		res.band = r->nwk_band;

		res.channel = r->nwk_channel;

		res.security = drv_to_bm(r->security_type);

		res.mfp = drv_to_bm_mfp(r->mfp_flag);

		memcpy(res.ssid,
		       r->ssid.nrf_wifi_ssid,
		       res.ssid_len);

		memcpy(res.bssid, r->mac_addr, NRF_WIFI_ETH_ADDR_LEN);

		if (r->signal.signal_type == NRF_WIFI_SIGNAL_TYPE_MBM) {
			int val = (r->signal.signal.mbm_signal);

			res.rssi = (val / 100);
		} else if (r->signal.signal_type == NRF_WIFI_SIGNAL_TYPE_UNSPEC) {
			res.rssi = (r->signal.signal.unspec_signal);
		}

		vif->scan_result_cb(&res);
		vif->scan_res_cnt++;
	}

	if (!more_res) {
		vif->scan_done = true;
		vif->scan_result_cb(NULL);
	}
}
#endif /* CONFIG_NRF70_RADIO_TEST */

int nrf70_fmac_init(void)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
#ifndef CONFIG_NRF70_RADIO_TEST
	struct nrf_wifi_fmac_callbk_fns callbk_fns = { 0 };
	struct nrf_wifi_data_config_params data_config = { 0 };
	struct rx_buf_pool_params rx_buf_pools[MAX_NUM_OF_RX_QUEUES] = { 0 };
	struct nrf70_wifi_vif_bm *vif = &nrf70_bm_priv.rpu_ctx_bm.vifs[0];
#endif /* CONFIG_NRF70_RADIO_TEST */
	unsigned int fw_ver = 0;
	void *rpu_ctx = nrf70_bm_priv.rpu_ctx_bm.rpu_ctx;
	struct nrf_wifi_tx_pwr_ctrl_params tx_pwr_ctrl_params = { 0 };
	/* TODO: Hardcoded to 10 dBm, take as parameter */
	struct nrf_wifi_tx_pwr_ceil_params tx_pwr_ceil_params = { 40 };
	struct nrf_wifi_board_params board_params = { 0 };
	bool enable_bf = false;
	enum op_band op_band = CONFIG_NRF_WIFI_OP_BAND;
#ifdef CONFIG_NRF_WIFI_LOW_POWER
	int sleep_type = -1;

#ifndef CONFIG_NRF700X_RADIO_TEST
	sleep_type = HW_SLEEP_ENABLE;
#else
	sleep_type = SLEEP_DISABLE;
#endif /* CONFIG_NRF700X_RADIO_TEST */
#endif /* CONFIG_NRF_WIFI_LOW_POWER */

#ifndef CONFIG_NRF70_RADIO_TEST
	NRF70_LOG_DBG("Initializing FMAC module in system mode");
#else
	NRF70_LOG_DBG("Initializing FMAC module in radio test mode");
#endif /* CONFIG_NRF70_RADIO_TEST */

#ifndef CONFIG_NRF70_RADIO_TEST
	vif->vif_idx = MAX_NUM_VIFS;

	/* Won't be used, but API requires it */
	memset(&rx_buf_pools, 0, sizeof(rx_buf_pools));
	rx_buf_pools[0].num_bufs = 2;
	rx_buf_pools[0].buf_sz = 1000;

	/* Regulator related call back functions */
	callbk_fns.reg_change_callbk_fn = reg_change_callbk_fn;
	//callbk_fns.event_get_reg = nrf_wifi_event_get_reg_zep;

	/* Scan related call back functions */
	callbk_fns.scan_start_callbk_fn = nrf_wifi_event_proc_scan_start_zep;
	callbk_fns.scan_done_callbk_fn = nrf_wifi_event_proc_scan_done_zep;
	//callbk_fns.scan_abort_callbk_fn = nrf_wifi_event_proc_scan_abort_zep;
	callbk_fns.disp_scan_res_callbk_fn = nrf_wifi_event_proc_disp_scan_res_zep;
#endif /* CONFIG_NRF70_RADIO_TEST */

#ifndef CONFIG_NRF70_RADIO_TEST
	// Initialize the FMAC module
	nrf70_bm_priv.fmac_priv = nrf_wifi_fmac_init(&data_config,
												 rx_buf_pools,
												 &callbk_fns);
#else
	nrf70_bm_priv.fmac_priv = nrf_wifi_fmac_init_rt();
#endif /* CONFIG_NRF70_RADIO_TEST */
	if (!nrf70_bm_priv.fmac_priv) {
		NRF70_LOG_ERR("Failed to initialize FMAC module\n");
		goto err;
	}

	rpu_ctx = nrf_wifi_fmac_dev_add(nrf70_bm_priv.fmac_priv,
									&nrf70_bm_priv.rpu_ctx_bm);
	if (!rpu_ctx) {
		NRF70_LOG_ERR("Failed to add device\n");
		goto deinit;
	}

	nrf70_bm_priv.rpu_ctx_bm.rpu_ctx = rpu_ctx;

	status = nrf_wifi_fw_load(rpu_ctx);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("Failed to load firmware\n");
		goto deinit;
	}

	status = nrf_wifi_fmac_ver_get(rpu_ctx, &fw_ver);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("Failed to get FMAC version\n");
		goto deinit;
	}

	NRF70_LOG_INF("Firmware (v%d.%d.%d.%d) booted successfully",
		NRF_WIFI_UMAC_VER(fw_ver),
		NRF_WIFI_UMAC_VER_MAJ(fw_ver),
		NRF_WIFI_UMAC_VER_MIN(fw_ver),
		NRF_WIFI_UMAC_VER_EXTRA(fw_ver));

#ifdef CONFIG_NRF_WIFI_BEAMFORMING
	enable_bf = true;
#endif

#ifndef CONFIG_NRF70_RADIO_TEST
	status = nrf_wifi_fmac_dev_init(rpu_ctx,
#ifdef CONFIG_NRF_WIFI_LOW_POWER
					sleep_type,
#endif /* CONFIG_NRF_WIFI_LOW_POWER */
					NRF_WIFI_DEF_PHY_CALIB,
					op_band,
					enable_bf,
					&tx_pwr_ctrl_params,
					&tx_pwr_ceil_params,
					&board_params);
#else
	status = nrf_wifi_fmac_dev_init_rt(rpu_ctx,
#ifdef CONFIG_NRF_WIFI_LOW_POWER
					sleep_type,
#endif /* CONFIG_NRF_WIFI_LOW_POWER */
					NRF_WIFI_DEF_PHY_CALIB,
					op_band,
					enable_bf,
					&tx_pwr_ctrl_params,
					&tx_pwr_ceil_params,
					&board_params);
#endif /* CONFIG_NRF70_RADIO_TEST */
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("Failed to initialize device\n");
		goto deinit;
	}

	NRF70_LOG_DBG("FMAC module initialized");

	return 0;
deinit:
	nrf70_fmac_deinit();
err:
	return -1;
}


enum nrf_wifi_status nrf_wifi_get_mac_addr(struct nrf70_wifi_vif_bm *vif)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	void *rpu_ctx = nrf70_bm_priv.rpu_ctx_bm.rpu_ctx;
	struct nrf_wifi_fmac_priv *fmac_priv = nrf70_bm_priv.fmac_priv;
	unsigned char mac_addr_str[18];
#ifdef CONFIG_NRF70_FIXED_MAC_ADDRESS_ENABLED
	int ret;
	char fixed_mac_addr[NR70_MAC_ADDR_LEN];

	ret = nrf_wifi_utils_hex_str_to_val(nrf70_bm_priv.fmac_priv->opriv,
					    CONFIG_WIFI_FIXED_MAC_ADDRESS,
					    fixed_mac_addr,
					    NR70_MAC_ADDR_LEN);
	if (ret < 0) {
		NR70_LOG_ERR("%s: Failed to parse MAC address: %s",
					 __func__,
					 CONFIG_WIFI_FIXED_MAC_ADDRESS);
		goto err;
	}

	memcpy(vif->mac_addr, fixed_mac_addr, NR70_MAC_ADDR_LEN);
#elif CONFIG_NRF70_OTP_MAC_ADDRESS
	status = nrf_wifi_fmac_otp_mac_addr_get(rpu_ctx,
											vif->vif_idx,
											vif->mac_addr);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: Fetching of MAC address from OTP failed",
			__func__);
		goto err;
	}
#endif

	nrf70_mac_txt(vif->mac_addr, mac_addr_str, sizeof(mac_addr_str));
	if (!nrf_wifi_utils_is_mac_addr_valid(fmac_priv->opriv,
	    vif->mac_addr)) {
		NRF70_LOG_ERR("%s: Invalid MAC address: %s",
			__func__,
			mac_addr_str);
		status = NRF_WIFI_STATUS_FAIL;
		memset(vif->mac_addr, 0, NR70_MAC_ADDR_LEN);
		goto err;
	}
	status = NRF_WIFI_STATUS_SUCCESS;
err:
	return status;
}

#ifndef CONFIG_NRF70_RADIO_TEST
#define STA_VIF_NAME "wlan0"
int nrf70_fmac_add_vif_sta(void)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	struct nrf_wifi_umac_add_vif_info add_vif_info;
	struct nrf_wifi_umac_chg_vif_state_info vif_info;
	void *rpu_ctx = nrf70_bm_priv.rpu_ctx_bm.rpu_ctx;
	struct nrf70_wifi_vif_bm *vif = &nrf70_bm_priv.rpu_ctx_bm.vifs[0];

	if (!rpu_ctx) {
		NRF70_LOG_ERR("%s: RPU context is NULL", __func__);
		return -1;
	}
	add_vif_info.iftype = NRF_WIFI_IFTYPE_STATION;

	memcpy(add_vif_info.ifacename, STA_VIF_NAME, strlen(STA_VIF_NAME));

	vif->vif_idx = nrf_wifi_fmac_add_vif(rpu_ctx, vif, &add_vif_info);
	if (vif->vif_idx >= MAX_NUM_VIFS) {
		NRF70_LOG_ERR("%s: FMAC returned invalid interface index", __func__);
		goto err;
	}

	status = nrf_wifi_get_mac_addr(vif);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: Failed to get MAC address", __func__);
		goto del_vif;
	}

	status = nrf_wifi_fmac_set_vif_macaddr(rpu_ctx, vif->vif_idx, vif->mac_addr);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: Failed to set MAC address", __func__);
		goto del_vif;
	}

	memset(&vif_info, 0, sizeof(vif_info));
	vif_info.state = NRF_WIFI_FMAC_IF_OP_STATE_UP;
	vif_info.if_index = vif->vif_idx;

	status = nrf_wifi_fmac_chg_vif_state(rpu_ctx, vif->vif_idx, &vif_info);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: Failed to set interface state", __func__);
		goto del_vif;
	}

	vif->op_state = NRF_WIFI_FMAC_IF_OP_STATE_UP;

	NRF70_LOG_DBG("STA interface added successfully");

	return 0;

del_vif:
	status = nrf_wifi_fmac_del_vif(rpu_ctx, vif->vif_idx);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: Failed to delete interface", __func__);
	}
err:
	return -1;
}


int nrf70_fmac_del_vif_sta(void)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	struct nrf_wifi_umac_chg_vif_state_info vif_info;
	void *rpu_ctx = nrf70_bm_priv.rpu_ctx_bm.rpu_ctx;
	struct nrf70_wifi_vif_bm *vif = &nrf70_bm_priv.rpu_ctx_bm.vifs[0];

	if (!rpu_ctx) {
		NRF70_LOG_ERR("%s: RPU context is NULL", __func__);
		return -1;
	}

	memset(&vif_info, 0, sizeof(vif_info));
	vif_info.state = NRF_WIFI_FMAC_IF_OP_STATE_DOWN;
	vif_info.if_index = vif->vif_idx;

	status = nrf_wifi_fmac_chg_vif_state(rpu_ctx, vif->vif_idx, &vif_info);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: Failed to set interface state", __func__);
		goto err;
	}

	status = nrf_wifi_fmac_del_vif(rpu_ctx, vif->vif_idx);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: Failed to delete interface", __func__);
		goto err;
	}

	NRF70_LOG_DBG("STA interface deleted successfully");

	return 0;
err:
	return -1;
}
#endif /* CONFIG_NRF70_RADIO_TEST */

int nrf70_fmac_deinit(void)
{
	NRF70_LOG_DBG("Deinitializing FMAC module");

#ifndef CONFIG_NRF70_RADIO_TEST
	nrf_wifi_fmac_dev_deinit(nrf70_bm_priv.rpu_ctx_bm.rpu_ctx);
	nrf_wifi_fmac_dev_rem(nrf70_bm_priv.rpu_ctx_bm.rpu_ctx);
	nrf_wifi_fmac_deinit(nrf70_bm_priv.fmac_priv);
#else
	nrf_wifi_fmac_deinit_rt(nrf70_bm_priv.fmac_priv);
	nrf_wifi_fmac_dev_rem_rt(nrf70_bm_priv.rpu_ctx_bm.rpu_ctx);
	nrf_wifi_fmac_deinit_rt(nrf70_bm_priv.fmac_priv);
#endif /* CONFIG_NRF70_RADIO_TEST */

	nrf70_bm_priv.fmac_priv = NULL;
	nrf70_bm_priv.rpu_ctx_bm.rpu_ctx = NULL;

	NRF70_LOG_DBG("FMAC module deinitialized");

	return 0;
}