/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file
 * @brief nRF70 Bare Metal initialization.
 */
#include "nrf70_bm_init.h"
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
	NRF70_LOG_DBG("Scan done event received");
}


static void nrf_wifi_event_proc_disp_scan_res_zep(void *vif_ctx,
				struct nrf_wifi_umac_event_new_scan_display_results *scan_res_event,
				unsigned int event_len,
				bool more_res)
{
	NRF70_LOG_DBG("Scan result event received");
}

int nrf70_fmac_init(void)
{
	enum nrf_wifi_status status = NRF_WIFI_STATUS_FAIL;
	struct nrf_wifi_fmac_callbk_fns callbk_fns = { 0 };
	struct nrf_wifi_data_config_params data_config = { 0 };
	struct rx_buf_pool_params rx_buf_pools[MAX_NUM_OF_RX_QUEUES];
	unsigned int fw_ver = 0;
	void *rpu_ctx = nrf70_bm_priv.rpu_ctx_bm.rpu_ctx;

	NRF70_LOG_DBG("Initializing FMAC module");

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

	// Initialize the FMAC module
	nrf70_bm_priv.fmac_priv = nrf_wifi_fmac_init(&data_config,
												 rx_buf_pools,
												 &callbk_fns);
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

	NRF70_LOG_DBG("FMAC module initialized");

	return 0;
deinit:
	nrf70_fmac_deinit();
err:
	return -1;
}

int nrf70_fmac_deinit(void)
{
	NRF70_LOG_DBG("Deinitializing FMAC module");

	nrf_wifi_fmac_dev_rem(nrf70_bm_priv.rpu_ctx_bm.rpu_ctx);
	nrf_wifi_fmac_deinit(nrf70_bm_priv.fmac_priv);

	nrf70_bm_priv.fmac_priv = NULL;
	nrf70_bm_priv.rpu_ctx_bm.rpu_ctx = NULL;

	NRF70_LOG_DBG("FMAC module deinitialized");

	return 0;
}