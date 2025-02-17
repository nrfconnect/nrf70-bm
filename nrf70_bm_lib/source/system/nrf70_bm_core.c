/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief nRF70 Bare Metal initialization.
 */
#include "system/nrf70_bm_core.h"
#include "system/nrf70_bm_lib.h"
#include "util.h"
#include "system/fmac_api.h"
#include "common/fmac_util.h"

struct nrf70_bm_sys_wifi_drv_priv nrf70_bm_sys_priv;
extern const struct nrf_wifi_osal_ops nrf_wifi_os_bm_ops;
INCBIN(_bin, nrf70_bm_sys_fw, STR(CONFIG_NRF_WIFI_SYS_FW_BIN));

#ifdef CONFIG_NRF70_RANDOM_MAC_ADDRESS
static void generate_random_mac_address(uint8_t *mac_addr)
{
	// For simplicty use Zephyr API to generate random number
	for (int i = 0; i < 6; i++) {
		mac_addr[i] = nrf_wifi_osal_rand8_get();
	}

	// Set the locally administered bit (bit 1 of the first byte)
	mac_addr[0] |= 0x02;

	// Clear the multicast bit (bit 0 of the first byte)
	mac_addr[0] &= 0xFE;
}
#endif /* CONFIG_WIFI_RANDOM_MAC_ADDRESS */

static enum nrf_wifi_status nrf70_bm_sys_get_mac_addr(struct nrf70_bm_sys_wifi_vif *vif,
						      uint8_t *mac_addr)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
#ifdef CONFIG_NRF70_OTP_MAC_ADDRESS
	void *rpu_ctx = nrf70_bm_sys_priv.rpu_ctx_bm.rpu_ctx;
#endif /* CONFIG_NRF70_OTP_MAC_ADDRESS */
	unsigned char mac_addr_str[13];
#ifdef CONFIG_NRF70_FIXED_MAC_ADDRESS_ENABLED
	int ret;
	char fixed_mac_addr[NR70_MAC_ADDR_LEN];
#endif /* CONFIG_NRF70_FIXED_MAC_ADDRESS_ENABLED */

	/* Runtime takes precedence over any other method */
	if (mac_addr) {
		memcpy(vif->mac_addr, mac_addr, NR70_MAC_ADDR_LEN);
		goto mac_addr_check;
	}

#ifdef CONFIG_NRF70_FIXED_MAC_ADDRESS_ENABLED
	if (strlen(CONFIG_NRF70_FIXED_MAC_ADDRESS) != 2 * NR70_MAC_ADDR_LEN) {
		NRF70_LOG_ERR("Invalid fixed MAC address format: len = %d",
			strlen(CONFIG_NRF70_FIXED_MAC_ADDRESS));
		return NRF_WIFI_STATUS_FAIL;
	}


	ret = nrf_wifi_utils_hex_str_to_val(
					    fixed_mac_addr,
					    NR70_MAC_ADDR_LEN,
					    CONFIG_NRF70_FIXED_MAC_ADDRESS);
	if (ret < 0) {
		NRF70_LOG_ERR("%s: Failed to parse MAC address: %s",
					 __func__,
					 CONFIG_NRF70_FIXED_MAC_ADDRESS);
		goto err;
	}

	memcpy(vif->mac_addr, fixed_mac_addr, NR70_MAC_ADDR_LEN);
#elif CONFIG_NRF70_RANDOM_MAC_ADDRESS
	generate_random_mac_address(vif->mac_addr);
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
mac_addr_check:
	nrf70_bm_sys_mac_txt(vif->mac_addr, mac_addr_str, sizeof(mac_addr_str));
	if (!nrf_wifi_utils_is_mac_addr_valid(vif->mac_addr)) {
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

static void reg_change_callbk_fn(void *vif_ctx,
				 struct nrf_wifi_event_regulatory_change *reg_change_event,
				 unsigned int event_len)
{
	struct nrf_wifi_fmac_dev_ctx *fmac_dev_ctx = NULL;

	(void)vif_ctx;
	(void)event_len;

	NRF70_LOG_DBG("Regulatory change event received");

	fmac_dev_ctx = nrf70_bm_sys_priv.rpu_ctx_bm.rpu_ctx;

	if (!fmac_dev_ctx) {
		NRF70_LOG_ERR("%s: Invalid FMAC device context", __func__);
		return;
	}

	if (!fmac_dev_ctx->waiting_for_reg_event) {
		NRF70_LOG_DBG("%s: Unsolicited regulatory change event", __func__);
		/* TODO: Handle unsolicited regulatory change event */
		return;
	}

	fmac_dev_ctx->reg_change = nrf_wifi_osal_mem_alloc(
		sizeof(struct nrf_wifi_event_regulatory_change));
	if (!fmac_dev_ctx->reg_change) {
		NRF70_LOG_ERR("%s: Failed to allocate memory for reg_change", __func__);
		return;
	}

	memcpy(fmac_dev_ctx->reg_change,
		   reg_change_event,
		   sizeof(struct nrf_wifi_event_regulatory_change));
	fmac_dev_ctx->reg_set_status = true;
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
	struct nrf70_bm_sys_wifi_vif *vif = vif_ctx;
	void *rpu_ctx = nrf70_bm_sys_priv.rpu_ctx_bm.rpu_ctx;
	NRF70_LOG_DBG("Scan done event received");

	status = nrf_wifi_sys_fmac_scan_res_get(rpu_ctx,
						vif->vif_idx,
						SCAN_DISPLAY);

	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: failed", __func__);
		goto err;
	}
err:
	return;
}

static inline enum nrf70_bm_sys_mfp_options drv_to_bm_mfp(unsigned char mfp_flag)
{
	if (!mfp_flag)
		return NRF70_MFP_DISABLE;
	if (mfp_flag & NRF_WIFI_MFP_REQUIRED)
		return NRF70_MFP_REQUIRED;
	if (mfp_flag & NRF_WIFI_MFP_CAPABLE)
		return NRF70_MFP_OPTIONAL;

	return NRF70_MFP_UNKNOWN;
}
static inline enum nrf70_bm_sys_security_type drv_to_bm_sec(int drv_security_type)
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
	struct nrf70_bm_sys_wifi_vif *vif = vif_ctx;
	uint16_t max_bss_cnt = 0;
	struct umac_display_results *r = NULL;
	struct nrf70_bm_sys_scan_result res;
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

		res.security = drv_to_bm_sec(r->security_type);

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


static void nrf70_bm_event_get_reg(void *vif_ctx,
				   struct nrf_wifi_reg *get_reg_event,
				   unsigned int event_len)
{
	struct nrf_wifi_fmac_dev_ctx *fmac_dev_ctx = NULL;

	(void)vif_ctx;
	(void)event_len;

	NRF70_LOG_DBG("%s: alpha2 = %c%c", __func__,
		   get_reg_event->nrf_wifi_alpha2[0],
		   get_reg_event->nrf_wifi_alpha2[1]);

	fmac_dev_ctx = nrf70_bm_sys_priv.rpu_ctx_bm.rpu_ctx;

	if (fmac_dev_ctx->alpha2_valid) {
		NRF70_LOG_ERR("%s: Unsolicited regulatory get!", __func__);
		return;
	}

	memcpy(&fmac_dev_ctx->alpha2,
		   &get_reg_event->nrf_wifi_alpha2,
		   sizeof(get_reg_event->nrf_wifi_alpha2));

	fmac_dev_ctx->reg_chan_count = get_reg_event->num_channels;
	memcpy(fmac_dev_ctx->reg_chan_info,
	       &get_reg_event->chn_info,
	       fmac_dev_ctx->reg_chan_count *
		   sizeof(struct nrf_wifi_get_reg_chn_info));

	fmac_dev_ctx->alpha2_valid = true;
}


int nrf70_bm_sys_fmac_init(void)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	struct nrf_wifi_fmac_callbk_fns callbk_fns = { 0 };
	struct nrf_wifi_data_config_params data_config = { 0 };
	struct rx_buf_pool_params rx_buf_pools[MAX_NUM_OF_RX_QUEUES] = { 0 };
	struct nrf70_bm_sys_wifi_vif *vif = &nrf70_bm_sys_priv.rpu_ctx_bm.vifs[0];
	unsigned int fw_ver = 0;
	void *rpu_ctx = nrf70_bm_sys_priv.rpu_ctx_bm.rpu_ctx;
	struct nrf_wifi_tx_pwr_ctrl_params tx_pwr_ctrl_params = { 0 };
	/* TODO: Hardcoded to 10 dBm, take as parameter */
	struct nrf_wifi_tx_pwr_ceil_params tx_pwr_ceil_params;
	struct nrf_wifi_board_params board_params = { 0 };
	bool enable_bf = false;
	enum op_band op_band = CONFIG_NRF_WIFI_OP_BAND;
#ifdef CONFIG_NRF_WIFI_LOW_POWER
	int sleep_type = HW_SLEEP_ENABLE;
#endif /* CONFIG_NRF_WIFI_LOW_POWER */

	NRF70_LOG_DBG("Initializing FMAC module in system mode");

	vif->vif_idx = MAX_NUM_VIFS;

	/* Won't be used, but API requires it */
	memset(&rx_buf_pools, 0, sizeof(rx_buf_pools));
	rx_buf_pools[0].num_bufs = 2;
	rx_buf_pools[0].buf_sz = 1000;

	/* Regulator related call back functions */
	callbk_fns.reg_change_callbk_fn = reg_change_callbk_fn;
	callbk_fns.event_get_reg = nrf70_bm_event_get_reg;

	/* Scan related call back functions */
	callbk_fns.scan_start_callbk_fn = nrf_wifi_event_proc_scan_start_zep;
	callbk_fns.scan_done_callbk_fn = nrf_wifi_event_proc_scan_done_zep;
	//callbk_fns.scan_abort_callbk_fn = nrf_wifi_event_proc_scan_abort_zep;
	callbk_fns.disp_scan_res_callbk_fn = nrf_wifi_event_proc_disp_scan_res_zep;

	nrf_wifi_osal_init(&nrf_wifi_os_bm_ops);

	// Initialize the FMAC module
	nrf70_bm_sys_priv.fmac_priv = nrf_wifi_sys_fmac_init(&data_config,
							 rx_buf_pools,
							 &callbk_fns);
	if (!nrf70_bm_sys_priv.fmac_priv) {
		NRF70_LOG_ERR("Failed to initialize FMAC module\n");
		goto err;
	}

	rpu_ctx = nrf_wifi_sys_fmac_dev_add(nrf70_bm_sys_priv.fmac_priv,
					   &nrf70_bm_sys_priv.rpu_ctx_bm);
	if (!rpu_ctx) {
		NRF70_LOG_ERR("Failed to add device\n");
		goto deinit;
	}

	nrf70_bm_sys_priv.rpu_ctx_bm.rpu_ctx = rpu_ctx;

	status = nrf70_bm_sys_fw_load(rpu_ctx);
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

	nrf70_bm_conf_tx_pwr_settings(&tx_pwr_ctrl_params,
				      &tx_pwr_ceil_params);

	nrf70_bm_conf_board_dep_params(&board_params);

#ifdef CONFIG_NRF_WIFI_BEAMFORMING
	enable_bf = true;
#endif

	status = nrf_wifi_sys_fmac_dev_init(rpu_ctx,
#ifdef CONFIG_NRF_WIFI_LOW_POWER
					    sleep_type,
#endif /* CONFIG_NRF_WIFI_LOW_POWER */
					    NRF_WIFI_DEF_PHY_CALIB,
					    op_band,
					    enable_bf,
					    &tx_pwr_ctrl_params,
					    &tx_pwr_ceil_params,
					    &board_params,
					    STR(CONFIG_NRF70_REG_DOMAIN));
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("Failed to initialize device\n");
		goto deinit;
	}

	NRF70_LOG_DBG("FMAC module initialized");

	return 0;
deinit:
	nrf_wifi_osal_deinit();
	nrf70_bm_sys_fmac_deinit();
err:
	return -1;
}

#define STA_VIF_NAME "wlan0"
int nrf70_bm_sys_fmac_add_vif_sta(uint8_t *mac_addr)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	struct nrf_wifi_umac_add_vif_info add_vif_info;
	struct nrf_wifi_umac_chg_vif_state_info vif_info;
	void *rpu_ctx = nrf70_bm_sys_priv.rpu_ctx_bm.rpu_ctx;
	struct nrf70_bm_sys_wifi_vif *vif = &nrf70_bm_sys_priv.rpu_ctx_bm.vifs[0];

	if (!rpu_ctx) {
		NRF70_LOG_ERR("%s: RPU context is NULL", __func__);
		return -1;
	}
	add_vif_info.iftype = NRF_WIFI_IFTYPE_STATION;

	memcpy(add_vif_info.ifacename, STA_VIF_NAME, strlen(STA_VIF_NAME));

	vif->vif_idx = nrf_wifi_sys_fmac_add_vif(rpu_ctx, vif, &add_vif_info);
	if (vif->vif_idx >= MAX_NUM_VIFS) {
		NRF70_LOG_ERR("%s: FMAC returned invalid interface index", __func__);
		goto err;
	}

	status = nrf70_bm_sys_get_mac_addr(vif, mac_addr);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: Failed to get MAC address", __func__);
		goto del_vif;
	}

	status = nrf_wifi_sys_fmac_set_vif_macaddr(rpu_ctx, vif->vif_idx, vif->mac_addr);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: Failed to set MAC address", __func__);
		goto del_vif;
	}

	NRF70_LOG_INF("MAC address set to %02X:%02X:%02X:%02X:%02X:%02X",
		vif->mac_addr[0], vif->mac_addr[1], vif->mac_addr[2],
		vif->mac_addr[3], vif->mac_addr[4], vif->mac_addr[5]);

	memset(&vif_info, 0, sizeof(vif_info));
	vif_info.state = NRF_WIFI_FMAC_IF_OP_STATE_UP;
	vif_info.if_index = vif->vif_idx;

	status = nrf_wifi_sys_fmac_chg_vif_state(rpu_ctx, vif->vif_idx, &vif_info);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: Failed to set interface state", __func__);
		goto del_vif;
	}

	vif->op_state = NRF_WIFI_FMAC_IF_OP_STATE_UP;

	NRF70_LOG_DBG("STA interface added successfully");

	return 0;

del_vif:
	status = nrf_wifi_sys_fmac_del_vif(rpu_ctx, vif->vif_idx);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: Failed to delete interface", __func__);
	}
err:
	return -1;
}


int nrf70_bm_sys_fmac_del_vif_sta(void)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	struct nrf_wifi_umac_chg_vif_state_info vif_info;
	void *rpu_ctx = nrf70_bm_sys_priv.rpu_ctx_bm.rpu_ctx;
	struct nrf70_bm_sys_wifi_vif *vif = &nrf70_bm_sys_priv.rpu_ctx_bm.vifs[0];

	if (!rpu_ctx) {
		NRF70_LOG_ERR("%s: RPU context is NULL", __func__);
		return -1;
	}

	memset(&vif_info, 0, sizeof(vif_info));
	vif_info.state = NRF_WIFI_FMAC_IF_OP_STATE_DOWN;
	vif_info.if_index = vif->vif_idx;

	status = nrf_wifi_sys_fmac_chg_vif_state(rpu_ctx, vif->vif_idx, &vif_info);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: Failed to set interface state", __func__);
		goto err;
	}

	status = nrf_wifi_sys_fmac_del_vif(rpu_ctx, vif->vif_idx);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: Failed to delete interface", __func__);
		goto err;
	}

	NRF70_LOG_DBG("STA interface deleted successfully");

	return 0;
err:
	return -1;
}

int nrf70_bm_sys_fmac_get_reg(struct nrf70_bm_regulatory_info *reg_info)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	void *rpu_ctx = nrf70_bm_sys_priv.rpu_ctx_bm.rpu_ctx;
	struct nrf_wifi_fmac_reg_info reg_info_fmac = { 0 };
	struct nrf70_bm_reg_chan_info *tmp_chan_info_out = NULL;
	struct nrf_wifi_get_reg_chn_info *tmp_chan_info_in = NULL;
	int chan_idx;

	if (!rpu_ctx) {
		NRF70_LOG_ERR("%s: RPU context is NULL", __func__);
		goto err;
	}

	status = nrf_wifi_fmac_get_reg(rpu_ctx, &reg_info_fmac);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: %d Failed to get regulatory info", __func__, status);
		goto err;
	}

	memcpy(reg_info->country_code, reg_info_fmac.alpha2, NRF_WIFI_COUNTRY_CODE_LEN);
	reg_info->num_channels = reg_info_fmac.reg_chan_count;

	if (!reg_info->chan_info) {
		NRF70_LOG_ERR("%s: Channel info buffer is NULL", __func__);
		goto err;
	}

	if (reg_info->num_channels > NRF70_MAX_CHANNELS) {
		NRF70_LOG_ERR("%s: Number of channels exceeds maximum supported (%d)",
			__func__, NRF70_MAX_CHANNELS);
		goto err;
	}

	for (chan_idx = 0; chan_idx < reg_info_fmac.reg_chan_count; chan_idx++) {
		tmp_chan_info_out = &(reg_info->chan_info[chan_idx]);
		tmp_chan_info_in = &(reg_info_fmac.reg_chan_info[chan_idx]);
		tmp_chan_info_out->center_frequency = tmp_chan_info_in->center_frequency;
		tmp_chan_info_out->dfs = !!tmp_chan_info_in->dfs;
		tmp_chan_info_out->max_power = tmp_chan_info_in->max_power;
		tmp_chan_info_out->passive_only = !!tmp_chan_info_in->passive_channel;
		tmp_chan_info_out->supported = !!tmp_chan_info_in->supported;
	}
	return 0;
err:
	return -1;
}

int nrf70_bm_sys_fmac_set_reg(struct nrf70_bm_regulatory_info *reg_info)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	void *rpu_ctx = nrf70_bm_sys_priv.rpu_ctx_bm.rpu_ctx;
	struct nrf_wifi_fmac_reg_info reg_info_fmac = { 0 };

	if (!rpu_ctx) {
		NRF70_LOG_ERR("%s: RPU context is NULL", __func__);
		goto err;
	}

	memcpy(reg_info_fmac.alpha2, reg_info->country_code, NRF_WIFI_COUNTRY_CODE_LEN);
	reg_info_fmac.force = reg_info->force;

	status = nrf_wifi_fmac_set_reg(rpu_ctx, &reg_info_fmac);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: Failed to set regulatory info", __func__);
		goto err;
	}

	return 0;
err:
	return -1;
}

int nrf70_bm_sys_fmac_deinit(void)
{
	NRF70_LOG_DBG("Deinitializing FMAC module");

	if (nrf70_bm_sys_priv.rpu_ctx_bm.rpu_ctx) {
		nrf_wifi_sys_fmac_dev_deinit(nrf70_bm_sys_priv.rpu_ctx_bm.rpu_ctx);
	}

	if (nrf70_bm_sys_priv.rpu_ctx_bm.rpu_ctx) {
		nrf_wifi_fmac_dev_rem(nrf70_bm_sys_priv.rpu_ctx_bm.rpu_ctx);
	}

	if (nrf70_bm_sys_priv.fmac_priv) {
		nrf_wifi_fmac_deinit(nrf70_bm_sys_priv.fmac_priv);
	}

	nrf_wifi_osal_deinit();
	nrf70_bm_sys_priv.fmac_priv = NULL;
	nrf70_bm_sys_priv.rpu_ctx_bm.rpu_ctx = NULL;

	NRF70_LOG_DBG("FMAC module deinitialized");

	return 0;
}

enum nrf_wifi_status nrf70_bm_sys_fw_load(void *rpu_ctx)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	const uint8_t *fw_start = NULL;
	const uint8_t *fw_end = NULL;

	fw_start = _bin_nrf70_bm_sys_fw_start;
	fw_end = _bin_nrf70_bm_sys_fw_end;

	status = nrf70_bm_fw_load(rpu_ctx, fw_start, fw_end);
	if (status != NRF_WIFI_STATUS_SUCCESS) {
		NRF70_LOG_ERR("%s: nrf_wifi_fw_load failed", __func__);
	}

	return status;
}
