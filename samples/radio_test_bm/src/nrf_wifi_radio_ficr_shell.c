/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/* @file
 * @brief NRF Wi-Fi radio test shell module
 */

#include "ficr_prog.h"
#include <nrf70_bm_core.h>
#include <nrf_wifi_radio_test_shell.h>
#include <util.h>

#include "fmac_api_common.h"

extern struct nrf70_wifi_drv_priv_bm nrf70_bm_priv;
static struct nrf70_wifi_ctx_bm *ctx = &nrf70_bm_priv.rpu_ctx_bm;

static void disp_location_status(char *region, unsigned int ret) {
  switch (ret) {

  case OTP_PROGRAMMED:
    RT_SHELL_PRINTF_INFO("%s Region is locked!!!\n", region);
    break;
  case OTP_ENABLE_PATTERN:
    RT_SHELL_PRINTF_INFO("%s Region is open for R/W\n", region);
    break;
  case OTP_FRESH_FROM_FAB:
    RT_SHELL_PRINTF_INFO("%s Region is unprogrammed - program to enable R/W\n",
                         region);
    break;
  default:
    RT_SHELL_PRINTF_ERROR("%s Region is in invalid state\n", region);
    break;
  }
  RT_SHELL_PRINTF_INFO("\n");
}

static void disp_fields_status(unsigned int flags) {
  if (flags & (~QSPI_KEY_FLAG_MASK)) {
    RT_SHELL_PRINTF_INFO("QSPI Keys are not programmed in OTP\n");
  } else {
    RT_SHELL_PRINTF_INFO("QSPI Keys are programmed in OTP\n");
  }

  if (flags & (~MAC0_ADDR_FLAG_MASK)) {
    RT_SHELL_PRINTF_INFO("MAC0 Address are not programmed in OTP\n");
  } else {
    RT_SHELL_PRINTF_INFO("MAC0 Address is programmed in OTP\n");
  }

  if (flags & (~MAC1_ADDR_FLAG_MASK)) {
    RT_SHELL_PRINTF_INFO("MAC1 Address are not programmed in OTP\n");
  } else {
    RT_SHELL_PRINTF_INFO("MAC1 Address is programmed in OTP\n");
  }

  if (flags & (~CALIB_XO_FLAG_MASK)) {
    RT_SHELL_PRINTF_INFO("CALIB_XO is not programmed in OTP\n");
  } else {
    RT_SHELL_PRINTF_INFO("CALIB_XO is programmed in OTP\n");
  }
}

static int nrf_wifi_radio_test_otp_get_status(size_t argc, const char *argv[]) {
  unsigned int ret, err;
  unsigned int val[OTP_MAX_WORD_LEN];

  /* read all the OTP memory */
  err = read_otp_memory(0, &val[0], OTP_MAX_WORD_LEN);
  if (err) {
    RT_SHELL_PRINTF_ERROR("FAILED reading otp memory......\n");
    return -ENOEXEC;
  }

  RT_SHELL_PRINTF_INFO("Checking OTP PROTECT Region......\n");
  ret = check_protection(&val[0], REGION_PROTECT, REGION_PROTECT + 1,
                         REGION_PROTECT + 2, REGION_PROTECT + 3);

  disp_location_status("OTP", ret);
  disp_fields_status(val[REGION_DEFAULTS]);

  return 0;
}

static int nrf_wifi_radio_test_otp_read_params(size_t argc, const char *argv[]) {
  unsigned int val[OTP_MAX_WORD_LEN];
  unsigned int ret, err;

  err = read_otp_memory(0, &val[0], OTP_MAX_WORD_LEN);
  if (err) {
    RT_SHELL_PRINTF_ERROR("FAILED reading otp memory......\n");
    return -ENOEXEC;
  }

  ret = check_protection(&val[0], REGION_PROTECT + 0, REGION_PROTECT + 1,
                         REGION_PROTECT + 2, REGION_PROTECT + 3);
  disp_location_status("OTP", ret);

  RT_SHELL_PRINTF_INFO("PRODTEST.FT.PROGVERSION = 0x%08x\n",
                       val[PRODTEST_FT_PROGVERSION]);
  RT_SHELL_PRINTF_INFO("\n");

  RT_SHELL_PRINTF_INFO("PRODTEST.TRIM0 = 0x%08x\n", val[PRODTEST_TRIM0]);
  RT_SHELL_PRINTF_INFO("PRODTEST.TRIM1 = 0x%08x\n", val[PRODTEST_TRIM1]);
  RT_SHELL_PRINTF_INFO("PRODTEST.TRIM2 = 0x%08x\n", val[PRODTEST_TRIM2]);
  RT_SHELL_PRINTF_INFO("PRODTEST.TRIM3 = 0x%08x\n", val[PRODTEST_TRIM3]);
  RT_SHELL_PRINTF_INFO("PRODTEST.TRIM4 = 0x%08x\n", val[PRODTEST_TRIM4]);
  RT_SHELL_PRINTF_INFO("PRODTEST.TRIM5 = 0x%08x\n", val[PRODTEST_TRIM5]);
  RT_SHELL_PRINTF_INFO("PRODTEST.TRIM6 = 0x%08x\n", val[PRODTEST_TRIM6]);
  RT_SHELL_PRINTF_INFO("PRODTEST.TRIM7 = 0x%08x\n", val[PRODTEST_TRIM7]);
  RT_SHELL_PRINTF_INFO("PRODTEST.TRIM8 = 0x%08x\n", val[PRODTEST_TRIM8]);
  RT_SHELL_PRINTF_INFO("PRODTEST.TRIM9 = 0x%08x\n", val[PRODTEST_TRIM9]);
  RT_SHELL_PRINTF_INFO("PRODTEST.TRIM10 = 0x%08x\n", val[PRODTEST_TRIM10]);
  RT_SHELL_PRINTF_INFO("PRODTEST.TRIM11 = 0x%08x\n", val[PRODTEST_TRIM11]);
  RT_SHELL_PRINTF_INFO("PRODTEST.TRIM12 = 0x%08x\n", val[PRODTEST_TRIM12]);
  RT_SHELL_PRINTF_INFO("PRODTEST.TRIM13 = 0x%08x\n", val[PRODTEST_TRIM13]);
  RT_SHELL_PRINTF_INFO("PRODTEST.TRIM14 = 0x%08x\n", val[PRODTEST_TRIM14]);
  RT_SHELL_PRINTF_INFO("\n");

  RT_SHELL_PRINTF_INFO("INFO.PART = 0x%08x\n", val[INFO_PART]);
  RT_SHELL_PRINTF_INFO("INFO.VARIANT = 0x%08x\n", val[INFO_VARIANT]);
  RT_SHELL_PRINTF_INFO("\n");

  RT_SHELL_PRINTF_INFO("INFO.UUID0 = 0x%08x\n", val[INFO_UUID + 0]);
  RT_SHELL_PRINTF_INFO("INFO.UUID1 = 0x%08x\n", val[INFO_UUID + 1]);
  RT_SHELL_PRINTF_INFO("INFO.UUID2 = 0x%08x\n", val[INFO_UUID + 2]);
  RT_SHELL_PRINTF_INFO("INFO.UUID3 = 0x%08x\n", val[INFO_UUID + 3]);
  RT_SHELL_PRINTF_INFO("\n");

  RT_SHELL_PRINTF_INFO("REGION.PROTECT0 = 0x%08x\n", val[REGION_PROTECT + 0]);
  RT_SHELL_PRINTF_INFO("REGION.PROTECT1 = 0x%08x\n", val[REGION_PROTECT + 1]);
  RT_SHELL_PRINTF_INFO("REGION.PROTECT2 = 0x%08x\n", val[REGION_PROTECT + 2]);
  RT_SHELL_PRINTF_INFO("REGION.PROTECT3 = 0x%08x\n", val[REGION_PROTECT + 3]);
  RT_SHELL_PRINTF_INFO("\n");

  if (val[REGION_PROTECT + 0] != OTP_FRESH_FROM_FAB) {
    RT_SHELL_PRINTF_INFO("MAC0.ADDRESS0 = 0x%08x\n", val[MAC0_ADDR]);
    RT_SHELL_PRINTF_INFO("MAC0.ADDRESS1 = 0x%08x\n", val[MAC0_ADDR + 1]);
    RT_SHELL_PRINTF_INFO(
        "MAC0.ADDRESS = %02x:%02x:%02x:%02x:%02x:%02x\n",
        (uint8_t)(val[MAC0_ADDR]), (uint8_t)(val[MAC0_ADDR] >> 8),
        (uint8_t)(val[MAC0_ADDR] >> 16), (uint8_t)(val[MAC0_ADDR] >> 24),
        (uint8_t)(val[MAC0_ADDR + 1]), (uint8_t)(val[MAC0_ADDR + 1] >> 8));
    RT_SHELL_PRINTF_INFO("\n");

    RT_SHELL_PRINTF_INFO("MAC1.ADDRESS0 = 0x%08x\n", val[MAC1_ADDR]);
    RT_SHELL_PRINTF_INFO("MAC1.ADDRESS1 = 0x%08x\n", val[MAC1_ADDR + 1]);
    RT_SHELL_PRINTF_INFO(
        "MAC1.ADDRESS = %02x:%02x:%02x:%02x:%02x:%02x\n",
        (uint8_t)(val[MAC1_ADDR]), (uint8_t)(val[MAC1_ADDR] >> 8),
        (uint8_t)(val[MAC1_ADDR] >> 16), (uint8_t)(val[MAC1_ADDR] >> 24),
        (uint8_t)(val[MAC1_ADDR + 1]), (uint8_t)(val[MAC1_ADDR + 1] >> 8));
    RT_SHELL_PRINTF_INFO("\n");

    RT_SHELL_PRINTF_INFO("CALIB.XO = 0x%02x\n", val[CALIB_XO] & 0xFF);

    RT_SHELL_PRINTF_INFO("REGION_DEFAULTS = 0x%08x\n", val[REGION_DEFAULTS]);
    RT_SHELL_PRINTF_INFO("\n");
  }
  return 0;
}

static int nrf_wifi_radio_test_otp_read_retrim_version(size_t argc,
                                                       const char *argv[]) {
  unsigned int val[1];
  unsigned int err;

  err = read_otp_memory(REGION_PROTECT, &val[0], 1);
  if (err) {
    RT_SHELL_PRINTF_ERROR("FAILED reading otp memory......\n");
    return -ENOEXEC;
  }

  if (val[0] == OTP_FRESH_FROM_FAB) {
    RT_SHELL_PRINTF_ERROR("Region is unprogrammed - program to enable R/W\n");
  } else {
    err = read_otp_memory(PRODRETEST_PROGVERSION, &val[0], 1);
    RT_SHELL_PRINTF_INFO("\nPRODRETEST.PROGVERSION = 0x%08x\n", val[0]);
    RT_SHELL_PRINTF_INFO("\n");
  }
  return 0;
}

static int nrf_wifi_radio_test_otp_read_retrim_params(size_t argc,
                                                      const char *argv[]) {
  unsigned int val[RETRIM_LEN];
  unsigned int err;

  err = read_otp_memory(REGION_PROTECT, val, 1);
  if (err) {
    RT_SHELL_PRINTF_ERROR("FAILED reading otp memory......\n");
    return -ENOEXEC;
  }

  if (val[0] == OTP_FRESH_FROM_FAB) {
    RT_SHELL_PRINTF_ERROR("Region is unprogrammed - program to enable R/W\n");
  } else {
    RT_SHELL_PRINTF_INFO("\n");
    err = read_otp_memory(PRODRETEST_TRIM0, val, RETRIM_LEN);
    for (int i = 0; i < RETRIM_LEN; i++) {
      RT_SHELL_PRINTF_INFO("PRODRETEST.TRIM%d = 0x%08x\n", i, val[i]);
    }
    RT_SHELL_PRINTF_INFO("\n");
  }

  return 0;
}

static int nrf_wifi_radio_test_otp_write_params(size_t argc, const char *argv[]) {
  unsigned int field;
  unsigned int write_val[20];
  unsigned int val[OTP_MAX_WORD_LEN];
  unsigned int ret, err;
  int status = 0;

  if (argc < 2) {
    RT_SHELL_PRINTF_ERROR("invalid # of args : %d\n", argc);
    return -ENOEXEC;
  }

  field = strtoul(argv[1], NULL, 0);
  /* Align to 32-bit word address */
  field >>= 2;

  if (field < REGION_PROTECT) {
    RT_SHELL_PRINTF_ERROR("INVALID Address 0x%x......\n", field << 2);
    return -ENOEXEC;
  }

  if ((field >= PRODRETEST_PROGVERSION) && (field <= PRODRETEST_TRIM14)) {
    RT_SHELL_PRINTF_ERROR("INVALID Address 0x%x......\n", field << 2);
    return -ENOEXEC;
  }

  err = read_otp_memory(REGION_PROTECT, &val[REGION_PROTECT], 4);
  if (err) {
    RT_SHELL_PRINTF_ERROR("FAILED reading otp memory......\n");
    return -ENOEXEC;
  }

  ret = check_protection(&val[0], REGION_PROTECT, REGION_PROTECT + 1,
                         REGION_PROTECT + 2, REGION_PROTECT + 3);
  disp_location_status("OTP", ret);
  if (ret != OTP_ENABLE_PATTERN && ret != OTP_FRESH_FROM_FAB) {
    RT_SHELL_PRINTF_ERROR("USER Region is not Writeable\n");
    return -ENOEXEC;
  }

  switch (field) {
  case REGION_PROTECT:
    if (argc != 3) {
      RT_SHELL_PRINTF_ERROR(
          "invalid # of args for REGION_PROTECT (expected 3) : %d\n", argc);
      return -ENOEXEC;
    }
    write_val[0] = strtoul(argv[2], NULL, 0);
    /* All consecutive 4 words of the REGION_PROTECT are written */
    write_otp_memory(REGION_PROTECT, &write_val[0]);
    break;
  case MAC0_ADDR:
  case MAC1_ADDR:
    struct nrf_wifi_fmac_dev_ctx *fmac_dev_ctx = ctx->rpu_ctx;

    if (!fmac_dev_ctx) {
      RT_SHELL_PRINTF_ERROR("Driver not initialized\n");
      return -ENOEXEC;
    }

    if (argc != 4) {
      RT_SHELL_PRINTF_ERROR(
          "invalid # of args for MAC ADDR write (expected 4) : %d\n", argc);
      return -ENOEXEC;
    }
    write_val[0] = strtoul(argv[2], NULL, 0);
    write_val[1] = strtoul(argv[3], NULL, 0) & 0xFFFF;

    if (!nrf_wifi_utils_is_mac_addr_valid(fmac_dev_ctx->fpriv->opriv,
                                          (const char *)&write_val[0])) {
      RT_SHELL_PRINTF_ERROR("Invalid MAC address. MAC address cannot be all "
                            "0's, broadcast or multicast address\n");
      return -ENOEXEC;
    }

    status = write_otp_memory(field, &write_val[0]);
    break;
  case CALIB_XO:
  case REGION_DEFAULTS:
    if (argc != 3) {
      RT_SHELL_PRINTF_ERROR(
          "invalid # of args for field %d (expected 3) : %d\n", field, argc);
      return -ENOEXEC;
    }
    write_val[0] = strtoul(argv[2], NULL, 0);
    status = write_otp_memory(field, &write_val[0]);
    break;
  case QSPI_KEY:
    if (argc != 6) {
      RT_SHELL_PRINTF_ERROR(
          "invalid # of args for QSPI_KEY (expected 6) : %d\n", argc);
      return -ENOEXEC;
    }
    write_val[0] = strtoul(argv[2], NULL, 0);
    write_val[1] = strtoul(argv[3], NULL, 0);
    write_val[2] = strtoul(argv[4], NULL, 0);
    write_val[3] = strtoul(argv[5], NULL, 0);
    /* All consecutive 4 words of the qspi keys are written now */
    status = write_otp_memory(QSPI_KEY, &write_val[0]);
    break;
  default:
    RT_SHELL_PRINTF_ERROR("unsupported field %d\n", field);
    return -ENOEXEC;
  }

  if (!status) {
    RT_SHELL_PRINTF_INFO("Finished Writing OTP params\n");
  }

  return 0;
}

static int nrf_wifi_radio_test_otp_write_retrim_version(size_t argc,
                                                        const char *argv[]) {
  unsigned int field;
  unsigned int write_val;
  unsigned int val[OTP_MAX_WORD_LEN];
  unsigned int ret, err;
  int status = 0;

  field = PRODRETEST_PROGVERSION;

  if (argc != 2) {
    RT_SHELL_PRINTF_ERROR("invalid # of args for field %d (expected 2) : %d\n",
                          field, argc);
    return -ENOEXEC;
  }

  err = read_otp_memory(REGION_PROTECT, &val[REGION_PROTECT], 4);
  if (err) {
    RT_SHELL_PRINTF_ERROR("FAILED reading otp memory......\n");
    return -ENOEXEC;
  }

  ret = check_protection(&val[0], REGION_PROTECT, REGION_PROTECT + 1,
                         REGION_PROTECT + 2, REGION_PROTECT + 3);
  disp_location_status("OTP", ret);
  if (ret != OTP_ENABLE_PATTERN && ret != OTP_FRESH_FROM_FAB) {
    RT_SHELL_PRINTF_ERROR("USER Region is not Writeable\n");
    return -ENOEXEC;
  }

  write_val = strtoul(argv[1], NULL, 0);
  status = write_otp_memory(field, &write_val);

  if (!status) {
    RT_SHELL_PRINTF_INFO("Finished Writing Retrim version\n");
  }

  return 0;
}

static int nrf_wifi_radio_test_otp_write_retrim_params(size_t argc,
                                                       const char *argv[]) {
  unsigned int index;
  unsigned int field;
  unsigned int write_val;
  unsigned int val[OTP_MAX_WORD_LEN];
  unsigned int ret, err;
  int status = 0;

  index = strtoul(argv[1], NULL, 0);
  field = index + PRODRETEST_TRIM0;

  if ((field < PRODRETEST_TRIM0) || (field > PRODRETEST_TRIM14)) {
    RT_SHELL_PRINTF_ERROR("INVALID Index %d......\n", index);
    return -ENOEXEC;
  }

  err = read_otp_memory(REGION_PROTECT, &val[REGION_PROTECT], 4);
  if (err) {
    RT_SHELL_PRINTF_ERROR("FAILED reading otp memory......\n");
    return -ENOEXEC;
  }

  ret = check_protection(&val[0], REGION_PROTECT, REGION_PROTECT + 1,
                         REGION_PROTECT + 2, REGION_PROTECT + 3);
  disp_location_status("OTP", ret);
  if (ret != OTP_ENABLE_PATTERN && ret != OTP_FRESH_FROM_FAB) {
    RT_SHELL_PRINTF_ERROR("USER Region is not Writeable\n");
    return -ENOEXEC;
  }

  switch (field) {
  case PRODRETEST_TRIM0:
  case PRODRETEST_TRIM1:
  case PRODRETEST_TRIM2:
  case PRODRETEST_TRIM3:
  case PRODRETEST_TRIM4:
  case PRODRETEST_TRIM5:
  case PRODRETEST_TRIM6:
  case PRODRETEST_TRIM7:
  case PRODRETEST_TRIM8:
  case PRODRETEST_TRIM9:
  case PRODRETEST_TRIM10:
  case PRODRETEST_TRIM11:
  case PRODRETEST_TRIM12:
  case PRODRETEST_TRIM13:
  case PRODRETEST_TRIM14:
    if (argc != 3) {
      RT_SHELL_PRINTF_ERROR(
          "invalid # of args for field %d (expected 3) : %d\n", field, argc);
      return -ENOEXEC;
    }
    write_val = strtoul(argv[2], NULL, 0);
    status = write_otp_memory(field, &write_val);
    break;
  default:
    RT_SHELL_PRINTF_ERROR("unsupported field %d\n", field);
    return -ENOEXEC;
  }

  if (!status) {
    RT_SHELL_PRINTF_INFO("Finished Writing Retrim param\n");
  }

  return 0;
}

#ifdef CONFIG_ZEPHYR_SHELL
DEFINE_CMD_HANDLER(nrf_wifi_radio_test_otp_get_status);
DEFINE_CMD_HANDLER(nrf_wifi_radio_test_otp_read_params);
DEFINE_CMD_HANDLER(nrf_wifi_radio_test_otp_read_retrim_version);
DEFINE_CMD_HANDLER(nrf_wifi_radio_test_otp_read_retrim_params);
DEFINE_CMD_HANDLER(nrf_wifi_radio_test_otp_write_params);
DEFINE_CMD_HANDLER(nrf_wifi_radio_test_otp_write_retrim_version);
DEFINE_CMD_HANDLER(nrf_wifi_radio_test_otp_write_retrim_params);

SHELL_STATIC_SUBCMD_SET_CREATE(
    nrf_wifi_radio_otp_subcmds,
    SHELL_CMD_ARG(otp_get_status, NULL, "Read OTP status",
                  RTSH(nrf_wifi_radio_test_otp_get_status), 1, 0),
    SHELL_CMD_ARG(
        otp_read_params, NULL,
        "Read User region status and information on programmed fields",
        RTSH(nrf_wifi_radio_test_otp_read_params), 1, 0),
    SHELL_CMD_ARG(otp_read_retrim_version, NULL, "Read Retrim Version",
                  RTSH(nrf_wifi_radio_test_otp_read_retrim_version), 1, 0),
    SHELL_CMD_ARG(otp_read_retrim_params, NULL, "Read Retrim Params",
                  RTSH(nrf_wifi_radio_test_otp_read_retrim_params), 1, 0),
    SHELL_CMD_ARG(otp_write_params, NULL,
                  "Write OTP Params\n"
                  "otp_write_params <addr offset> [arg1] [arg2]...[argN]",
                  RTSH(nrf_wifi_radio_test_otp_write_params), 2, 16),
    SHELL_CMD_ARG(otp_write_retrim_version, NULL,
                  "Write OTP Retrim Version\n"
                  "otp_write_retrim_version <retrim version>",
                  RTSH(nrf_wifi_radio_test_otp_write_retrim_version), 2, 0),
    SHELL_CMD_ARG(otp_write_retrim_params, NULL,
                  "Write OTP Retrim Params\n"
                  "otp_write_params <retrim index> [retrim data]",
                  RTSH(nrf_wifi_radio_test_otp_write_retrim_params), 3, 0),
    SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(wifi_radio_ficr_prog, &nrf_wifi_radio_otp_subcmds,
                   "nRF Wi-Fi radio FICR commands", NULL);

#endif

static int nrf_wifi_radio_otp_shell_init(void) { return 0; }

SYS_INIT(nrf_wifi_radio_otp_shell_init, APPLICATION,
         CONFIG_APPLICATION_INIT_PRIORITY);
