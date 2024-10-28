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

int execute(char *command, char *result);
uint64_t execute_int(const char* cmd);
char* gpon_get_equipment_id();
uint64_t gpon_get_unicast_packets_sent();
uint64_t gpon_get_unicast_packets_received();
uint64_t gpon_get_discard_packets_sent();
uint64_t gpon_get_discard_packets_received();
uint64_t gpon_get_multicast_packets_sent();
uint64_t gpon_get_multicast_packets_received();
uint64_t gpon_get_broadcast_packets_sent();
uint64_t gpon_get_broadcast_packets_received();
uint64_t gpon_get_eth_packets_sent();
uint64_t gpon_get_eth_packets_received();
uint64_t gpon_get_eth_errors_sent();
uint64_t gpon_get_eth_errors_received();
char* gpon_get_rxp_signal_level();
char* gpon_get_txp_signal_level();
char* gpon_get_current_voltage();
char* gpon_get_current_bias();
char* gpon_get_current_temperature();
char* gpon_get_nbr_upstream();
char* gpon_get_pm_modulefirmwareversion();
char* gpon_get_PM_status();
char* gpon_get_PM_ponmode();
char* gpon_get_pm_operationalstate();
char* gpon_get_ploam_vendorid();
char* gpon_get_ploam_serialnumber();
uint64_t gpon_get_onu_ani_stats(const char* keyword);
uint64_t gpon_get_ani_stats_packets_sent();
uint64_t gpon_get_ani_stats_packets_received();
uint64_t gpon_get_ani_stats_discard_packets_sent();
uint64_t gpon_get_ani_stats_discard_packets_received();
uint64_t gpon_get_onu_ani_tc_pm(const char* keyword);
uint64_t gpon_get_ploam_micerrors();
uint64_t gpon_get_downstream_message_count();
uint64_t gpon_get_ranging_time();
uint64_t gpon_get_upstream_message_count();
uint64_t gpon_get_omci_baselinemessagecount();
uint64_t gpon_get_omci_extendedmessagecount();
uint64_t gpon_get_omci_micerrors();
uint64_t gpon_get_ploam_onuid();
uint64_t gpon_get_gem_directions();
int gpon_get_PM_alam_los();
int gpon_get_PM_alam_lof();
uint64_t gpon_get_PM_alam_lcdg();
int gpon_get_pm_alarm_tf();

#endif
