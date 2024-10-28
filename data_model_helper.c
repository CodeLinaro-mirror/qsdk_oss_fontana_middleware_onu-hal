/*
*
* Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: ISC
*
*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdint.h>
#include "data_model_helper.h"
#include <inttypes.h>

uint64_t res;
char retValue[256] = {'\0'};

int execute(char *command, char *result)
{
    FILE *fp = NULL;
    char output[256] = {'\0'};
    char *str = NULL,*pos=NULL;
    fp = popen(command, "r");
    if(NULL == fp)
    {
        printf("Failed to run command\n" );
        return -1;
    }
    
    size_t bytesRead = fread(output, 1, sizeof(output) - 1, fp);
    if (bytesRead == 0) {
        pclose(fp);
        return -1;
    } else {
        output[bytesRead] = '\0'; // Null-terminate the buffer
    }
    // fgets(output, sizeof(output)-1, fp);
    if (NULL != (pos=strchr(output, '\n')) ) {
        *pos = '\0';
    }
    pclose(fp);
    snprintf(result, sizeof(output), "%s", output);
    return 0;
}

uint64_t execute_int(const char* cmd)
{
    FILE *fp = NULL;
    char output[256] = {'\0'};
    uint64_t res;
    char *str = NULL,*pos=NULL;
    fp = popen(cmd, "r");
    if(NULL == fp)
    {
        printf("Failed to run command\n" );
        return -1;
    }
    size_t bytesRead = fread(output, 1, sizeof(output) - 1, fp);
    if (bytesRead == 0) {
        pclose(fp);
        return -1;
    } else {
        output[bytesRead] = '\0'; 
    }
    sscanf(output, "%" SCNu64, &res);
    pclose(fp);
    return res;
}



uint64_t gpon_get_unicast_packets_sent()
{
    uint64_t result;
    result=execute_int("omci -c 'switch pkt pkt' | awk '/ifIn/,/ifOut/ {if ($2 ~ /^[0-9]+$/) sum += $3} END {print sum}'");
    return result;
}

uint64_t gpon_get_unicast_packets_received()
{
    uint64_t result;
    result=execute_int("omci -c 'switch pkt pkt' | awk '/ifOut/,0 {if ($2 ~ /^[0-9]+$/) sum += $3} END {print sum}'");
    return result;
}

uint64_t gpon_get_discard_packets_sent()
{
    uint64_t result;
    result=execute_int("omci -c 'switch pkt pkt' | awk '/ifOut/,0 {if ($2 ~ /^[0-9]+$/) sum += $6} END {print sum}'");
    return result;
}

uint64_t gpon_get_discard_packets_received()
{
    uint64_t result;
    result=execute_int("omci -c 'switch pkt pkt' | awk '/ifOut/,0 {if ($2 ~ /^[0-9]+$/) sum += $6} END {print sum}'");
    return result;
}

uint64_t gpon_get_multicast_packets_sent()
{
    uint64_t result;
    result=execute_int("omci -c 'switch pkt pkt' | awk '/ifOut/,0 {if ($2 ~ /^[0-9]+$/) sum += $4} END {print sum}'");
    return result;
}

uint64_t gpon_get_multicast_packets_received()
{
    uint64_t result;
    result=execute_int("omci -c 'switch pkt pkt' | awk '/ifOut/,0 {if ($2 ~ /^[0-9]+$/) sum += $4} END {print sum}'");
    return result;
}

uint64_t gpon_get_broadcast_packets_sent()
{
    uint64_t result;
    result=execute_int("omci -c 'switch pkt pkt' | awk '/ifOut/,0 {if ($2 ~ /^[0-9]+$/) sum += $5} END {print sum}'");
    return result;
}

uint64_t gpon_get_broadcast_packets_received()
{
    uint64_t result;
    result=execute_int("omci -c 'switch pkt pkt' | awk '/ifOut/,0 {if ($2 ~ /^[0-9]+$/) sum += $5} END {print sum}'");
    return result;
}

uint64_t gpon_get_eth_packets_sent()
{
    uint64_t result;
    result=execute_int("omci -c 'switch pkt' | awk '{sum += $2} END {print sum}'");
    return result;
}

uint64_t gpon_get_eth_packets_received()
{
    uint64_t result;
    result=execute_int("omci -c 'switch pkt' | awk '{sum += $3} END {print sum}'");
    return result;
}

uint64_t gpon_get_eth_errors_sent()
{
    uint64_t result;
    result=execute_int("omci -c 'switch pkt' | awk '{sum += $6} END {print sum}'");
    return result;
}

uint64_t gpon_get_eth_errors_received()
{
    uint64_t result;
    result=execute_int("omci -c 'switch pkt' | awk '{sum += $7} END {print sum}'");
    return result;
}

// xpon_onu.{i}.ani.{i}.transceiver.{i} - nominal_bit_rate_upstream
char* gpon_get_nbr_upstream()
{
    char sv[256] = {'\0'};
    int ret = RETURN_ERR, itemp;
    ret = execute("cat /proc/trex/gpon/global | grep 'us_rate' | cut -d'(' -f2 | cut -d')' -f1", sv);
    if(RETURN_OK != ret)
    {
        printf("\nError %s\n", __func__);
    }
    else
    {
        memset(retValue, 0, 256);
        strlcpy(retValue, sv, strlen(sv) + 1);
        return retValue;
    }
    return "-1";

}

// xpon_onu.{i} - equipment id
char* gpon_get_equipment_id()
{
    char sv[256] = {'\0'};
    int ret = RETURN_ERR, itemp;
    ret = execute("omci -c 'mib me 257' | grep Equipment_id | awk '{print $3}'", sv);
    if(RETURN_OK != ret)
    {
        printf("\nError %s\n", __func__);
    }
    else
    {
        memset(retValue, 0, 256);
        strlcpy(retValue, sv, strlen(sv) + 1);
        return retValue;
    }
    return "-1";
}

// xpon_onu.{i} - name
char* gpon_get_onu_name()
{
    char sv[256] = {'\0'};
    int ret = RETURN_ERR, itemp;
    ret = execute("omci -c 'mib me 256' | grep Serial_number | cut -d' ' -f4 | tr -d ','", sv);
    if(RETURN_OK != ret)
    {
        printf("\nError %s\n", __func__);
    }
    else
    {
        memset(retValue, 0, 256);
        strlcpy(retValue, sv, strlen(sv) + 1);
        return retValue;
    }
    return "-1";
}

// xpon_onu.{i}.ani.{i}.transceiver.{i} - rx_power
char* gpon_get_rxp_signal_level()
{
    char sv[256] = {'\0'};
    int ret = RETURN_ERR, itemp;
    ret = execute("omci -c 'info anig' | grep rx | awk '{print $2 $3 $4}'", sv);
    if(RETURN_OK != ret)
    {
        printf("\nError %s\n", __func__);
    }
    else
    {
        memset(retValue, 0, 256);
        strlcpy(retValue, sv, strlen(sv) + 1);
        return retValue;
    }
    return "-1";
}

// xpon_onu.{i}.ani.{i}.transceiver.{i} - tx_power
char* gpon_get_txp_signal_level()
{
    char sv[256] = {'\0'};
    int ret = RETURN_ERR, itemp;
    ret = execute("omci -c 'info anig' | grep tx | awk '{print $2 $3 $4}'", sv);
    if(RETURN_OK != ret)
    {
        printf("\nError %s\n", __func__);
    }
    else
    {
        memset(retValue, 0, 256);
        strlcpy(retValue, sv, strlen(sv) + 1);
        return retValue;
    }
    return "-1";
}

// xpon_onu.{i}.ani.{i}.transceiver.{i} - voltage
char* gpon_get_current_voltage()
{
    char sv[256] = {'\0'};
    int ret = RETURN_ERR, itemp;
    ret = execute("omci -c 'info anig' | grep vcc | awk '{print $2 $3}'", sv);
    if(RETURN_OK != ret)
    {
        printf("\nError %s\n", __func__);
    }
    else
    {
        memset(retValue, 0, 256);
        strlcpy(retValue, sv, strlen(sv) + 1);
        return retValue;
    }
    return "-1";
}

// xpon_onu.{i}.ani.{i}.transceiver.{i} - bias
char* gpon_get_current_bias()
{
    char sv[256] = {'\0'};
    int ret = RETURN_ERR, itemp;
    ret = execute("omci -c 'info anig' | grep bias | awk '{print $2 $3}'", sv);
    if(RETURN_OK != ret)
    {
        printf("\nError %s\n", __func__);
    }
    else
    {
        memset(retValue, 0, 256);
        strlcpy(retValue, sv, strlen(sv) + 1);
        return retValue;
    }
    return "-1";
}

// xpon_onu.{i}.ani.{i}.transceiver.{i} - temperature
char* gpon_get_current_temperature()
{
    char sv[256] = {'\0'};
    int ret = RETURN_ERR, itemp;
    ret = execute("omci -c 'info anig' | grep degree | awk '{print $2 $3}'", sv);
    if(RETURN_OK != ret)
    {
        printf("\nError %s\n", __func__);
    }
    else
    {
        memset(retValue, 0, 256);
        strlcpy(retValue, sv, strlen(sv) + 1);
        return retValue;
    }
    return "-1";
}

// xpon_onu.{i} - version
// xpon_onu.{i}.software_image.{i} - version
// xpon_onu.{i}.ani.{i}.transceiver.{i} - module_version
char* gpon_get_pm_modulefirmwareversion()
{
    char sv[256] = {'\0'};
    int ret = RETURN_ERR, itemp;
    ret = execute("cat /etc/openwrt_version", sv);
    if(RETURN_OK != ret)
    {
        printf("\nError %s\n", __func__);
    }
    else
    {
        memset(retValue, 0, 256);
        strlcpy(retValue, sv, strlen(sv) + 1);
        return retValue;
    }
    return "-1";
}

// xpon_onu.{i}.ani.{i} - status
char* gpon_get_PM_status()
{
    char sv[256] = {'\0'};
    int ret = RETURN_ERR;
    
    ret = execute("cat /proc/trex/gpon/onu | grep -w state | cut -c 7-8", sv);
    if(RETURN_OK != ret)
    {
        printf("\nError %s\n", __func__);
    }
    else
    {
        if(strcmp(sv,"O1") == 0)
	   return "Down";
        else if(strcmp(sv,"O5") == 0)
	   return "Up";
        else
	   return "Not Present";

    }
    return "-1";
}

// xpon_onu.{i}.ani.{i} - pon_mode
// xpon_onu.{i}.ani.{i}.transceiver.{i} - pon_mode
char* gpon_get_PM_ponmode()
{
    char sv[256] = {'\0'};
    int ret = RETURN_ERR, itemp;
    ret = execute("cat /proc/trex/gpon/global | grep -w gpon_mode | awk '{print $2}' | cut -d '(' -f2- | cut -d ')' -f1", sv);
    if(RETURN_OK != ret)
    {
        printf("\nError %s\n", __func__);
    }
    else
    {
        memset(retValue, 0, 256);
        strlcpy(retValue, sv, strlen(sv) + 1);
        return retValue;
    }
    return "-1";
}

// xpon_onu.{i}.ani.{i}.tc.onu_activation - onu_state
char*  gpon_get_pm_operationalstate()
{
    char sv[256] = {'\0'};
    int res = RETURN_ERR;
    int ret = RETURN_ERR;

    ret = execute("cat /proc/trex/gpon/onu | grep -w state | cut -c 7-8", sv);
    if(RETURN_OK != ret)
    {
        printf("\nError %s\n", __func__);
        //return "-1";
    }
    else
    {
        printf("\n11:%s=%s", __func__, sv);
        res=strcmp(sv,"O5");
        if(res==0)
           return "Active";
        else
           return "Inactive";
    }
    return "-1";
}

// xpon_onu.{i}.ani.{i}.tc.onu_activation - vendor_id
char* gpon_get_ploam_vendorid()
{
    char sv[256] = {'\0'};
    int ret = RETURN_ERR, itemp;
    ret = execute("cat /proc/trex/gpon/onu | grep 'sn' | cut -d'=' -f2- | cut -c 1-4", sv);
    if(RETURN_OK != ret)
    {
        printf("\nError %s\n", __func__);
    }
    else
    {
        memset(retValue, 0, 256);
        strlcpy(retValue, sv, strlen(sv) + 1);
        printf("sreehari: retvalue=%s\n", retValue);
        return retValue;
    }
    return "-1";
}

// xpon_onu.{i}.ani.{i}.tc.onu_activation - serial_number
char* gpon_get_ploam_serialnumber()
{
    char sv[256] = {'\0'};
    int ret = RETURN_ERR, itemp;
    ret = execute("cat /proc/trex/gpon/onu | grep 'sn' | cut -d'=' -f2-", sv);
    if(RETURN_OK != ret)
    {
        printf("\nError %s\n", __func__);
    }
    else
    {
        memset(retValue, 0, 256);
        strlcpy(retValue, sv, strlen(sv) + 1);
        return retValue;
    }
    return "-1";
}

// xpon_onu.{i}.ani.{i}.stats - 
// bytes_sent, bytes_received, unicast_packets_sent, unicast_packets_received, 
// multicast_packets_sent, multicast_packets_received, broadcast_packets_sent, broadcast_packets_received
uint64_t gpon_get_onu_ani_stats(const char* keyword)
{
    FILE *fp = NULL;
    char output[256] = {'\0'};
    uint64_t res;
    char *str = NULL,*pos=NULL;
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "cat /proc/trex/pmhd/l2dsrv | awk '{for(i=1;i<=NF;i++) if($i==\"%s\") print $(i+1)}'", keyword);
    fp = popen(cmd, "r");
    if(NULL == fp)
    {
        printf("Failed to run command\n" );
        return -1;
    }
    size_t bytesRead = fread(output, 1, sizeof(output) - 1, fp);
    if (bytesRead == 0) {
        pclose(fp);
        return -1;
    } else {
        output[bytesRead] = '\0'; 
    }
    sscanf(output, "%" SCNu64, &res);
    pclose(fp);
    return res;
}

uint64_t gpon_get_ani_stats_packets_sent()
{
    uint64_t result;
    result = (gpon_get_onu_ani_stats("us_ucast") + gpon_get_onu_ani_stats("us_bcast") + gpon_get_onu_ani_stats("us_mcast")) - gpon_get_onu_ani_stats("us_drop_mem_ov") - gpon_get_onu_ani_stats("us_drop_lkp_ov") - gpon_get_onu_ani_stats("us_drop_treat") - gpon_get_onu_ani_stats("rx_fcs_err") - gpon_get_onu_ani_stats("rx_mac_err") - gpon_get_onu_ani_stats("rx_pause");
    return result;
}

uint64_t gpon_get_ani_stats_packets_received()
{
    uint64_t result;
    result = (gpon_get_onu_ani_stats("ds_ucast") + gpon_get_onu_ani_stats("ds_bcast") + gpon_get_onu_ani_stats("ds_mcast"));
    return result;
}

uint64_t gpon_get_ani_stats_discard_packets_sent()
{
    uint64_t result;
    result = (gpon_get_onu_ani_stats("us_drop_mem_ov") + gpon_get_onu_ani_stats("us_drop_lkp_ov") + gpon_get_onu_ani_stats("us_drop_treat"));
    return result;
}

uint64_t gpon_get_ani_stats_discard_packets_received()
{
    uint64_t result;
    result = (gpon_get_onu_ani_stats("ds_drop_mem_ov") + gpon_get_onu_ani_stats("ds_drop_lkp_ov") + gpon_get_onu_ani_stats("ds_drop_treat"));
    return result;
}

// xpon_onu.{i}.ani.{i}.tc.pm.phy - 
// corrected_fec_bytes, corrected_fec_codewords, uncorrectable_fec_codewords
// total_fec_codewords, psbd_hec_error_count, 
uint64_t gpon_get_onu_ani_tc_pm(const char* keyword)
{
    FILE *fp = NULL;
    char output[256] = {'\0'};
    uint64_t res;
    char *str = NULL,*pos=NULL;
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "cat /proc/trex/pmhd/gpon | awk '{for(i=1;i<=NF;i++) if($i==\"%s\") print $(i+1)}'", keyword);
    fp = popen(cmd, "r");
    if(NULL == fp)
    {
        printf("Failed to run command\n" );
        return -1;
    }
    size_t bytesRead = fread(output, 1, sizeof(output) - 1, fp);
    if (bytesRead == 0) {
        pclose(fp);
        return -1;
    } else {
        output[bytesRead] = '\0'; 
    }
    sscanf(output, "%" SCNu64, &res);
    pclose(fp);
    return res;
}


// ****************************************
// xpon_onu.{i}.ani.{i}.tc.pm.ploam

uint64_t gpon_get_ploam_micerrors()
{
    uint64_t result;
    result=execute_int("cat /proc/trex/pmhd/gpon | grep -w ploam_mic_err | awk '{print $3}'");
    return result;
}

uint64_t gpon_get_downstream_message_count()
{
    // uint64_t result;
    // result=execute_int("");
    return -1;
}

uint64_t gpon_get_ranging_time()
{
    uint64_t result;
    result=execute_int("cat /proc/trex/pmhd/gpon | grep ranging_time_msg_cnt | awk '{print $3}'");
    return result;
}

uint64_t gpon_get_upstream_message_count()
{
    // uint64_t result;
    // result=execute_int("");
    return -1;
}
// ****************************************


// ****************************************
// xpon_onu.{i}.ani.{i}.tc.pm.omci

uint64_t gpon_get_omci_baselinemessagecount()
{
    uint64_t result;
    result=execute_int("cat /proc/trex/sys/omci | grep -w 'omci rx' -A 6 | grep -w 'dev_type_a' | awk '{print$2}'");
    return result;
}

uint64_t gpon_get_omci_extendedmessagecount()
{
    uint64_t result;
    result=execute_int("cat /proc/trex/sys/omci | grep -w 'omci rx' -A 6 | grep -w 'dev_type_b' | awk '{print$2}'");
    return result;
}

uint64_t gpon_get_omci_micerrors()
{
    uint64_t result;
    result=execute_int("cat /proc/trex/sys/omci | grep -w 'mic_error' | awk '{print$2}'");
    return result;
}

// ****************************************


// ****************************************
// xpon_onu.{i}.ani.{i}.tc.gem.port.{i}

uint64_t gpon_get_ploam_onuid()
{
    uint64_t result;
    result=execute_int("cat /proc/trex/gpon/onu | grep -w id  | cut -d':' -f2- | cut -d '(' -f1");
    return result;
}

uint64_t gpon_get_gem_directions()
{
    return -1;
}

// *********************************


// ****************************************
// xpon_onu.{i}.ani.{i}.tc.alarms

int gpon_get_pm_alarm_tf()
{
    float num;
    char sv[256] = {'\0'};
    int ret = RETURN_ERR, itemp;
    ret = execute("omci -c 'info anig' | grep tx | awk '{print $2}'", sv);
    if(RETURN_OK != ret)
    {
        printf("\nError %s\n", __func__);
    }
    else
    {
        if (sscanf(sv, "%f", &num) == 1)
        {
            if (num>1995.3 && num<6309.6)
            {
                return 0;
            }
            else
            {
                return 1;
            }
        }
    }
    return -1;
}

int gpon_get_PM_alam_los()
{
    uint64_t result;
    result=execute_int("grep -w rx_los /proc/trex/io/pon | awk '{print $2}'");
    if (result == 1)
    {
        return 1;
    }
    return 0;
}

int gpon_get_PM_alam_lof()
{
    char sv[256] = {'\0'};
    int ret = RETURN_ERR, itemp;
    ret = execute("cat /proc/trex/gpon/onu | grep -w state | cut -c 7-8", sv);
    if(RETURN_OK != ret)
    {
        printf("\nError %s\n", __func__);
    }
    if (sv == "O1")
    {
        return 1;
    }
    return 0;
}

uint64_t gpon_get_PM_alam_lcdg()
{
    uint64_t result;
    result=execute_int("cat /proc/trex/pmhd/gpon | grep -w xgem_hd_hec_err  | awk '{print$7}'");
    return result;
}


