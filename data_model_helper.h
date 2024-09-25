/*
*
* Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: ISC
*
*/
#ifndef _HELPER_H
#define _HELPER_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
// #include "json_hal_common.h"

#define CHAR char
#define RETURN_ERR -1
#define RETURN_OK 0

extern char retValue[256];
extern char snbuf[256];

static int execute(char *command, char *result);
uint64_t execute_int(const char* cmd);
char* platform_hal_Get_pm_modulefirmwareversion();
char* platform_hal_Get_PM_status();
char* platform_hal_Get_PM_ponmode();
char* platform_hal_get_pm_operationalstate();
char* platform_hal_get_ploam_vendorid();
char* platform_hal_get_ploam_serialnumber();
uint64_t gpon_get_onu_ani_stats(const char* keyword);
uint64_t gpon_get_onu_ani_tc_pm(const char* keyword);


#endif
