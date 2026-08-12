/*
*
* Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: ISC
*
*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "data_model.h"
#include "data_model_helper.h"
#include <stdlib.h> /* calloc() */
#include <string.h> /* strdup() */

#include "onu_hal_macros.h"
#include "onu_hal_trace.h"

static const char XPON_ONU[] = "xpon_onu.x";
static const char SW_IMG_1[] = "xpon_onu.x.software_image.1";
static const char SW_IMG_2[] = "xpon_onu.x.software_image.2";
// Dynamic ethernet UNI definitions - will be allocated at runtime
static char** eth_uni_names = NULL;
static char** eth_uni_stats_names = NULL;
static int max_eth_uni_count = 0;
static const char ANI_1[] = "xpon_onu.x.ani.1";
static const char ANI_STATS[] = "xpon_onu.x.ani.1.stats";
static const char ONU_ACTIVATION[] = "xpon_onu.x.ani.1.tc.onu_activation";
static const char ANU_TC_PM_PHY[] = "xpon_onu.x.ani.1.tc.pm.phy";
static const char TC_PM_GEM[] = "xpon_onu.x.ani.1.tc.pm.gem";
static const char TC_PM_PLOAM[] = "xpon_onu.x.ani.1.tc.pm.ploam";
static const char TC_PM_OMCI[] = "xpon_onu.x.ani.1.tc.pm.omci";
static const char TC_GEM[] = "xpon_onu.x.ani.1.tc.gem";
static const char GEM_PORT_1[] = "xpon_onu.x.ani.1.tc.gem.port.1";
static const char GEM_PORT_2[] = "xpon_onu.x.ani.1.tc.gem.port.2";
static const char GEM_PORT_3[] = "xpon_onu.x.ani.1.tc.gem.port.3";
static const char GEM_PORT_4[] = "xpon_onu.x.ani.1.tc.gem.port.4";
static const char GEM_PORT_5[] = "xpon_onu.x.ani.1.tc.gem.port.5";
static const char GEM_PORT_6[] = "xpon_onu.x.ani.1.tc.gem.port.6";
static const char GEM_PORT_7[] = "xpon_onu.x.ani.1.tc.gem.port.7";
static const char GEM_PORT_8[] = "xpon_onu.x.ani.1.tc.gem.port.8";
static const char PM_GEM_PORT_1[] = "xpon_onu.x.ani.1.tc.gem.port.1.pm";
static const char PM_GEM_PORT_2[] = "xpon_onu.x.ani.1.tc.gem.port.2.pm";
static const char PM_GEM_PORT_3[] = "xpon_onu.x.ani.1.tc.gem.port.3.pm";
static const char PM_GEM_PORT_4[] = "xpon_onu.x.ani.1.tc.gem.port.4.pm";
static const char PM_GEM_PORT_5[] = "xpon_onu.x.ani.1.tc.gem.port.5.pm";
static const char PM_GEM_PORT_6[] = "xpon_onu.x.ani.1.tc.gem.port.6.pm";
static const char PM_GEM_PORT_7[] = "xpon_onu.x.ani.1.tc.gem.port.7.pm";
static const char PM_GEM_PORT_8[] = "xpon_onu.x.ani.1.tc.gem.port.8.pm";
static const char PERF_THRESHOLDS[] = "xpon_onu.x.ani.1.tc.performance_thresholds";
static const char ALARMS[] = "xpon_onu.x.ani.1.tc.alarms";
static const char TRANSCEIVER_1[] = "xpon_onu.x.ani.1.transceiver.1";
static const char TRANSCEIVER_2[] = "xpon_onu.x.ani.1.transceiver.2";

static const char XPON_ONU_ONE[] = "xpon_onu.1";
static const char XPON_ONU_TWO[] = "xpon_onu.2";

static const char METHOD_GET[] = "get";
static const char METHOD_ENABLE[] = "enable";
static const char METHOD_DISABLE[] = "disable";
static const char METHOD_GET_PARAMS[] = "get_params";

/* To test dm:object-changed notification */
static uint32_t s_transceiver_1_vendor_rev = 1;
static uint32_t s_onu_state = 2;


typedef enum _obj_id {
    id_xpon_onu       = 0,
    id_sw_img_1,
    id_sw_img_2,
    id_ani_1,
    id_ani_stats,
    id_onu_activation,
    id_anu_tc_pm_phy,
    id_tc_pm_gem,
    id_tc_pm_ploam,
    id_tc_pm_omci,
    id_tc_gem,
    id_gem_port_1,
    id_gem_port_2,
    id_gem_port_3,
    id_gem_port_4,
    id_gem_port_5,
    id_gem_port_6,
    id_gem_port_7,
    id_gem_port_8,
    id_pm_gem_port_1,
    id_pm_gem_port_2,
    id_pm_gem_port_3,
    id_pm_gem_port_4,
    id_pm_gem_port_5,
    id_pm_gem_port_6,
    id_pm_gem_port_7,
    id_pm_gem_port_8,
    id_perf_thresholds,
    id_alarms,
    id_transceiver_1,
    id_transceiver_2,
    n_static_objects  
} obj_id_t;

// Dynamic ethernet UNI objects will be allocated separately
static object_wrapper_t** s_eth_uni_objects = NULL;
static object_wrapper_t** s_eth_uni_stats_objects = NULL;

// Forward declarations for helper functions
static int extract_uni_index_from_name(const char* name);
static int is_ethernet_uni_object(const char* name);
static int is_ethernet_uni_stats_object(const char* name);
static object_wrapper_t* create_eth_uni_object_wrapper(const char* path, int is_stats);
static int dm_init_ethernet_uni_objects(int onu_nr);

typedef struct _object_info {
    obj_id_t id;
    const char* name;
    const struct ubus_method* methods;
    uint32_t n_methods;
} object_info_t;

enum {
    PARAM_NAME,
    MAX_PARAMS
};

static int32_t s_rx_power = -2000;
static int32_t s_tx_power = -3000;
static uint32_t s_voltage = 0;
static uint32_t s_bias = 0;
static int32_t s_temperature = -274;

static int common_method_handler(struct ubus_context* ctx, struct ubus_object* obj,
                                 struct ubus_request_data* req, const char* method,
                                 UNUSED struct blob_attr* msg);

static const struct ubus_method METHODS_GET_ONLY[] = {
    { .name = METHOD_GET, .handler = common_method_handler, .policy = NULL, .n_policy = 0 }
};

static const struct ubus_method METHODS_GET_ENABLE_AND_DISABLE[] = {
    { .name = METHOD_GET, .handler = common_method_handler, .policy = NULL, .n_policy = 0 },
    { .name = METHOD_ENABLE, .handler = common_method_handler, .policy = NULL, .n_policy = 0 },
    { .name = METHOD_DISABLE, .handler = common_method_handler, .policy = NULL, .n_policy = 0 }
};

static const struct blobmsg_policy GET_PARAMS_POLICY[] = {
    { .name = "names", .type = BLOBMSG_TYPE_STRING }
};

static struct ubus_method METHODS_TRANSCEIVER[] = {
    { .name = METHOD_GET, .handler = common_method_handler, .policy = NULL, .n_policy = 0 },
    { .name = METHOD_GET_PARAMS, .handler = common_method_handler, .policy = GET_PARAMS_POLICY, .n_policy = 1 }
};

static const object_info_t OBJECT_INFO[n_static_objects] = {
    {
        .id = id_xpon_onu,
        .name = XPON_ONU,
        .methods = METHODS_GET_ENABLE_AND_DISABLE,
        .n_methods = ARRAY_SIZE(METHODS_GET_ENABLE_AND_DISABLE)
    },
    {
        .id = id_sw_img_1,
        .name = SW_IMG_1,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_sw_img_2,
        .name = SW_IMG_2,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_ani_1,
        .name = ANI_1,
        .methods = METHODS_GET_ENABLE_AND_DISABLE,
        .n_methods = ARRAY_SIZE(METHODS_GET_ENABLE_AND_DISABLE)
    },
    {
        .id = id_ani_stats,
        .name = ANI_STATS,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_onu_activation,
        .name = ONU_ACTIVATION,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_anu_tc_pm_phy,
        .name = ANU_TC_PM_PHY,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_tc_pm_gem,
        .name = TC_PM_GEM,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_tc_pm_ploam,
        .name = TC_PM_PLOAM,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_tc_pm_omci,
        .name = TC_PM_OMCI,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_tc_gem,
        .name = TC_GEM,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_gem_port_1,
        .name = GEM_PORT_1,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_gem_port_2,
        .name = GEM_PORT_2,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_gem_port_3,
        .name = GEM_PORT_3,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_gem_port_4,
        .name = GEM_PORT_4,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_gem_port_5,
        .name = GEM_PORT_5,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_gem_port_6,
        .name = GEM_PORT_6,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_gem_port_7,
        .name = GEM_PORT_7,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_gem_port_8,
        .name = GEM_PORT_8,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_pm_gem_port_1,
        .name = PM_GEM_PORT_1,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_pm_gem_port_2,
        .name = PM_GEM_PORT_2,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_pm_gem_port_3,
        .name = PM_GEM_PORT_3,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_pm_gem_port_4,
        .name = PM_GEM_PORT_4,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_pm_gem_port_5,
        .name = PM_GEM_PORT_5,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_pm_gem_port_6,
        .name = PM_GEM_PORT_6,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_pm_gem_port_7,
        .name = PM_GEM_PORT_7,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_pm_gem_port_8,
        .name = PM_GEM_PORT_8,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_perf_thresholds,
        .name = PERF_THRESHOLDS,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_alarms,
        .name = ALARMS,
        .methods = METHODS_GET_ONLY,
        .n_methods = ARRAY_SIZE(METHODS_GET_ONLY)
    },
    {
        .id = id_transceiver_1,
        .name = TRANSCEIVER_1,
        .methods = METHODS_TRANSCEIVER,
        .n_methods = ARRAY_SIZE(METHODS_TRANSCEIVER)
    },
    {
        .id = id_transceiver_2,
        .name = TRANSCEIVER_2,
        .methods = METHODS_TRANSCEIVER,
        .n_methods = ARRAY_SIZE(METHODS_TRANSCEIVER)
    }
};

static const char DIGITS[] = { '0', '1', '2' };

static struct blob_buf b;

static struct ubus_context* s_ctx = NULL;
static int s_onu_nr = 1;

static bool s_onu_enabled = true;
static bool s_ani_one_enabled = true;


typedef struct _request {
    struct ubus_request_data req;
    struct uloop_timeout timeout;
    char* obj_name;
    char* method_name;
    char* param_name;
} request_t;

static object_wrapper_t* s_objects[n_static_objects];


static void request_delete(request_t* req) {
    when_null(req, exit);
    free(req->obj_name);
    free(req->method_name);
    free(req->param_name);
    free(req);
exit:
    return;
}

static bool str_equal(const char* const str1, const char* const str2, size_t str1_len) {
    return ((str1_len == strlen(str2)) &&
            (strncmp(str1, str2, str1_len) == 0)) ? true : false;
}

static void data_model_for_get_method(const char* name) {
    const size_t name_len = strlen(name);
    char path[128];
    snprintf(path, 128, "%s", name);
    const int index_of_onu_nr = strlen("xpon_onu.");
    path[index_of_onu_nr] = 'x';
    /**
     * 'path' has same length as 'name'. So code below can use 'name_len'
     * for both.
     */

    if(str_equal(path, XPON_ONU, name_len)) 
    {
        SAH_TRACE_INFO("path='%s' => XPON_ONU", path);
#ifdef BOARD_HERMOSA
        /* Hermosa (ipq52xx): run script ONCE, parse all fields in-memory.*/
        char onu_index[4] = "1";
        const size_t idx_pos = strlen("xpon_onu.");
        if(strlen(name) > idx_pos)
        {
            onu_index[0] = name[idx_pos];
            onu_index[1] = '\0';
        }
        gpon_hermosa_run_script("getXponOnuContent", onu_index);
        blobmsg_add_u8(&b, "enable", s_onu_enabled ? 1 : 0);
        blobmsg_add_string(&b, "version", gpon_hermosa_parse_field("Version"));
        blobmsg_add_string(&b, "equipment_id", gpon_hermosa_parse_field("EquipmentId"));
        blobmsg_add_u32(&b, "software_number_of_entries", (uint32_t)gpon_hermosa_parse_field_int("SoftwareImageNumberOfEntries"));
        blobmsg_add_u32(&b, "ethernet_uni_number_of_entries", (uint32_t)gpon_hermosa_parse_field_int("EthernetUNINumberOfEntries"));
        blobmsg_add_u32(&b, "ani_number_of_entries", (uint32_t)gpon_hermosa_parse_field_int("ANINumberOfEntries"));
        blobmsg_add_string(&b, "name", gpon_hermosa_parse_field("Name"));
#else
        /* Marina / Alder / Miami*/
        blobmsg_add_u8(&b, "enable", s_onu_enabled ? 1 : 0);
        blobmsg_add_string(&b, "version", gpon_get_pm_modulefirmwareversion());
        blobmsg_add_string(&b, "equipment_id", gpon_get_equipment_id());
        blobmsg_add_u32(&b, "software_number_of_entries", gpon_get_software_number_of_entries());
        blobmsg_add_u32(&b, "ethernet_uni_number_of_entries", gpon_get_eth_uni_number_of_entries());
        blobmsg_add_u32(&b, "ani_number_of_entries", 1);
        if(str_equal(name, XPON_ONU_ONE, name_len)) 
        {
            SAH_TRACE_INFO("name='%s' => XPON_ONU_ONE", name);
            blobmsg_add_string(&b, "name", "ONU_ONE");
        } 
        else if(str_equal(name, XPON_ONU_TWO, name_len)) 
        {
            SAH_TRACE_INFO("name='%s' => XPON_ONU_TWO", name);
            blobmsg_add_string(&b, "name", "ONU_TWO");
        }
#endif
    }
    
    else if(str_equal(path, SW_IMG_1, name_len)) 
    {
        SAH_TRACE_INFO("path='%s' => SW_IMG_1", path);
        blobmsg_add_u32(&b, "id", 0);
        blobmsg_add_u8(&b, "is_committed", gpon_get_sw_img_is_committed(0));
        blobmsg_add_u8(&b, "is_active", gpon_get_sw_img_is_active(0));
        blobmsg_add_u8(&b, "is_valid", gpon_get_sw_img_is_valid(0));
        blobmsg_add_string(&b, "version", gpon_get_sw_img_version(0));
    } 
    
    else if(str_equal(path, SW_IMG_2, name_len)) 
    {
        SAH_TRACE_INFO("path='%s' => SW_IMG_2", path);
        blobmsg_add_u32(&b, "id", 1);
        blobmsg_add_u8(&b, "is_committed", gpon_get_sw_img_is_committed(1));
        blobmsg_add_u8(&b, "is_active", gpon_get_sw_img_is_active(1));
        blobmsg_add_u8(&b, "is_valid", gpon_get_sw_img_is_valid(1));
        blobmsg_add_string(&b, "version", gpon_get_sw_img_version(1));
    }
    
    // Handle dynamic ethernet UNI objects
    else if (is_ethernet_uni_object(name)) {
        int uni_index = extract_uni_index_from_name(name);
        if (uni_index > 0 && uni_index <= max_eth_uni_count) {
#ifdef BOARD_HERMOSA
            /* Hermosa: EthernetUNI build script */
            char uni_script_index[8];
            char onu_nr_char = name[strlen("xpon_onu.")];
            snprintf(uni_script_index, sizeof(uni_script_index), "%c.%d", onu_nr_char, uni_index);

            if (is_ethernet_uni_stats_object(name)) {
                /* Hermosa EthernetUNI Stats*/
                SAH_TRACE_INFO("path='%s' => ETH_UNI_%d_STATS (Hermosa)", path, uni_index);
                gpon_hermosa_run_script("getXponOnuEthernetUNIStatsContent", uni_script_index);
                blobmsg_add_u64(&b, "bytes_sent",                  gpon_hermosa_parse_field_int("BytesSent"));
                blobmsg_add_u64(&b, "bytes_received",              gpon_hermosa_parse_field_int("BytesReceived"));
                blobmsg_add_u64(&b, "packets_sent",                gpon_hermosa_parse_field_int("PacketsSent"));
                blobmsg_add_u64(&b, "packets_received",            gpon_hermosa_parse_field_int("PacketsReceived"));
                blobmsg_add_u64(&b, "errors_sent",                 gpon_hermosa_parse_field_int("ErrorsSent"));
                blobmsg_add_u64(&b, "errors_received",             gpon_hermosa_parse_field_int("ErrorsReceived"));
                blobmsg_add_u64(&b, "unicast_packets_sent",        gpon_hermosa_parse_field_int("UnicastPacketsSent"));
                blobmsg_add_u64(&b, "unicast_packets_received",    gpon_hermosa_parse_field_int("UnicastPacketsReceived"));
                blobmsg_add_u64(&b, "discard_packets_sent",        gpon_hermosa_parse_field_int("DiscardPacketsSent"));
                blobmsg_add_u64(&b, "discard_packets_received",    gpon_hermosa_parse_field_int("DiscardPacketsReceived"));
                blobmsg_add_u64(&b, "multicast_packets_sent",      gpon_hermosa_parse_field_int("MulticastPacketsSent"));
                blobmsg_add_u64(&b, "multicast_packets_received",  gpon_hermosa_parse_field_int("MulticastPacketsReceived"));
                blobmsg_add_u64(&b, "broadcast_packets_sent",      gpon_hermosa_parse_field_int("BroadcastPacketsSent"));
                blobmsg_add_u64(&b, "broadcast_packets_received",  gpon_hermosa_parse_field_int("BroadcastPacketsReceived"));
                blobmsg_add_u32(&b, "unknown_proto_packets_received", 0);
            } else {
                /* Hermosa EthernetUNI Content*/
                SAH_TRACE_INFO("path='%s' => ETH_UNI_%d (Hermosa)", path, uni_index);
                gpon_hermosa_run_script("getXponOnuEthernetUNIContent", uni_script_index);
                const char* en_val = gpon_hermosa_parse_field("Enable");
                blobmsg_add_u8(&b, "enable", (strcmp(en_val, "true") == 0) ? 1 : 0);
                blobmsg_add_string(&b, "name",             gpon_hermosa_parse_field("Name"));
                blobmsg_add_string(&b, "alias",            gpon_hermosa_parse_field("Alias"));
                blobmsg_add_string(&b, "status",           gpon_hermosa_parse_field("Status"));
                blobmsg_add_string(&b, "ani_list",         gpon_hermosa_parse_field("ANIs"));
                blobmsg_add_u32(&b, "last_change", 0);
                blobmsg_add_string(&b, "lower_layers",     "");
                blobmsg_add_string(&b, "interdomain_id",   gpon_hermosa_parse_field("InterdomainID"));
                blobmsg_add_string(&b, "interdomain_name", gpon_hermosa_parse_field("InterdomainName"));
            }
#else
            /* Marina / Alder / Miami*/
            if (is_ethernet_uni_stats_object(name)) {
                SAH_TRACE_INFO("path='%s' => ETH_UNI_%d_STATS", path, uni_index);
                blobmsg_add_u32(&b, "bytes_sent", 0);
                blobmsg_add_u32(&b, "bytes_received", 0);
                blobmsg_add_u64(&b, "packets_sent", gpon_get_eth_packets_sent_by_index(uni_index - 1));
                blobmsg_add_u64(&b, "packets_received", gpon_get_eth_packets_received_by_index(uni_index - 1));
                blobmsg_add_u64(&b, "errors_sent", gpon_get_eth_errors_sent_by_index(uni_index - 1));
                blobmsg_add_u64(&b, "errors_received", gpon_get_eth_errors_received_by_index(uni_index - 1));
                blobmsg_add_u64(&b, "unicast_packets_sent", gpon_get_unicast_packets_sent_by_index(uni_index - 1));
                blobmsg_add_u64(&b, "unicast_packets_received", gpon_get_unicast_packets_received_by_index(uni_index - 1));
                blobmsg_add_u64(&b, "discard_packets_sent", gpon_get_discard_packets_sent_by_index(uni_index - 1));
                blobmsg_add_u64(&b, "discard_packets_received", gpon_get_discard_packets_received_by_index(uni_index - 1));
                blobmsg_add_u64(&b, "multicast_packets_sent", gpon_get_multicast_packets_sent_by_index(uni_index - 1));
                blobmsg_add_u64(&b, "multicast_packets_received", gpon_get_multicast_packets_received_by_index(uni_index - 1));
                blobmsg_add_u64(&b, "broadcast_packets_sent", gpon_get_broadcast_packets_sent_by_index(uni_index - 1));
                blobmsg_add_u64(&b, "broadcast_packets_received", gpon_get_broadcast_packets_received_by_index(uni_index - 1));
                blobmsg_add_u32(&b, "unknown_proto_packets_received", 0);
            } else {
                SAH_TRACE_INFO("path='%s' => ETH_UNI_%d", path, uni_index);
                char alias[64];
                snprintf(alias, sizeof(alias), "cpe-EthernetUNI-%d", uni_index);
                blobmsg_add_u8(&b, "enable", gpon_get_xpon_eth_uni_enable_by_index(uni_index - 1));
                blobmsg_add_string(&b, "name",  gpon_get_eth_uni_name_by_index(uni_index - 1));
                blobmsg_add_string(&b, "alias", alias);
                blobmsg_add_string(&b, "status",  gpon_get_eth_uni_status_by_index(uni_index - 1));
                blobmsg_add_string(&b, "ani_list",  gpon_get_eth_uni_ani_list_by_index(uni_index - 1));
                blobmsg_add_u32(&b, "last_change", 0);
                blobmsg_add_string(&b, "lower_layers", "");
                blobmsg_add_string(&b, "interdomain_id",  gpon_get_eth_uni_interdomain_id_by_index(uni_index - 1));
                blobmsg_add_string(&b, "interdomain_name", " ");
            }
#endif
        } else {
            SAH_TRACE_ERROR("Invalid ethernet UNI index: %d", uni_index);
        }
    }
    
    else if(str_equal(path, ANI_1, name_len)) 
    {
        SAH_TRACE_INFO("path='%s' => ANI_1", path);
#ifdef BOARD_HERMOSA
        /* Hermosa ANI Data Model*/
        char ani_script_index[8];
        char onu_nr_char = name[strlen("xpon_onu.")];
        snprintf(ani_script_index, sizeof(ani_script_index), "%c.1", onu_nr_char);
        gpon_hermosa_run_script("getXponOnuAniContent", ani_script_index);
        blobmsg_add_u8(&b, "enable", s_ani_one_enabled ? 1 : 0);
        blobmsg_add_string(&b, "name", gpon_hermosa_parse_field("Name"));
        blobmsg_add_string(&b, "alias", gpon_hermosa_parse_field("Alias"));
        blobmsg_add_u64(&b, "last_change", gpon_hermosa_parse_field_int("LastChange"));
        blobmsg_add_string(&b, "status", gpon_hermosa_parse_field("Status"));
        blobmsg_add_string(&b, "pon_mode", gpon_hermosa_parse_field("PONMode"));
        blobmsg_add_u32(&b, "transceiver_number_of_entries", (uint32_t)gpon_hermosa_parse_field_int("TransceiverNumberOfEntries"));
#else
        /* Marina / Alder / Miami*/
        blobmsg_add_u8(&b, "enable", s_ani_one_enabled ? 1 : 0);
        blobmsg_add_string(&b, "name", "MyANI");
        blobmsg_add_string(&b, "alias", "cpe-ANI-1");
        blobmsg_add_u64(&b, "last_change", gpon_get_onu_ani_last_change());
        blobmsg_add_string(&b, "status", gpon_get_PM_status());
        blobmsg_add_string(&b, "pon_mode", gpon_get_PM_ponmode());
        blobmsg_add_u32(&b, "transceiver_number_of_entries", 1);
#endif
    } 
    
    else if(str_equal(path, ANI_STATS, name_len))
    {
        SAH_TRACE_INFO("path='%s' => ANI_STATS", path);
#ifdef BOARD_HERMOSA
        {
            char ani_script_index[8];
            char onu_nr_char = name[strlen("xpon_onu.")];
            snprintf(ani_script_index, sizeof(ani_script_index), "%c.1", onu_nr_char);
            gpon_hermosa_run_script("getXponOnuAniStatsContent", ani_script_index);
            blobmsg_add_u64(&b, "bytes_sent",                  gpon_hermosa_parse_field_int("BytesSent"));
            blobmsg_add_u64(&b, "bytes_received",              gpon_hermosa_parse_field_int("BytesReceived"));
            blobmsg_add_u64(&b, "packets_sent",                gpon_hermosa_parse_field_int("PacketsSent"));
            blobmsg_add_u64(&b, "packets_received",            gpon_hermosa_parse_field_int("PacketsReceived"));
            blobmsg_add_u64(&b, "errors_sent",                 gpon_hermosa_parse_field_int("ErrorsSent"));
            blobmsg_add_u64(&b, "errors_received",             gpon_hermosa_parse_field_int("ErrorsReceived"));
            blobmsg_add_u64(&b, "unicast_packets_sent",        gpon_hermosa_parse_field_int("UnicastPacketsSent"));
            blobmsg_add_u64(&b, "unicast_packets_received",    gpon_hermosa_parse_field_int("UnicastPacketsReceived"));
            blobmsg_add_u64(&b, "discard_packets_sent",        gpon_hermosa_parse_field_int("DiscardPacketsSent"));
            blobmsg_add_u64(&b, "discard_packets_received",    gpon_hermosa_parse_field_int("DiscardPacketsReceived"));
            blobmsg_add_u64(&b, "multicast_packets_sent",      gpon_hermosa_parse_field_int("MulticastPacketsSent"));
            blobmsg_add_u64(&b, "multicast_packets_received",  gpon_hermosa_parse_field_int("MulticastPacketsReceived"));
            blobmsg_add_u64(&b, "broadcast_packets_sent",      gpon_hermosa_parse_field_int("BroadcastPacketsSent"));
            blobmsg_add_u64(&b, "broadcast_packets_received",  gpon_hermosa_parse_field_int("BroadcastPacketsReceived"));
            blobmsg_add_u64(&b, "unknown_proto_packets_received", 0);
        }
#else
        /* Marina / Alder / Miami*/
        blobmsg_add_u64(&b, "bytes_sent", gpon_get_onu_ani_stats("us_byte_out-"));
        blobmsg_add_u64(&b, "bytes_received", gpon_get_onu_ani_stats("ds_byte_in-"));
        blobmsg_add_u64(&b, "packets_sent", gpon_get_ani_stats_packets_sent());
        blobmsg_add_u64(&b, "packets_received", gpon_get_ani_stats_packets_received());
        blobmsg_add_u64(&b, "errors_sent", gpon_get_ani_errors_sent());
        blobmsg_add_u64(&b, "errors_received", gpon_get_ani_errors_received());
        blobmsg_add_u64(&b, "unicast_packets_sent", gpon_get_onu_ani_stats("us_ucast"));
        blobmsg_add_u64(&b, "unicast_packets_received", gpon_get_onu_ani_stats("ds_ucast"));
        blobmsg_add_u64(&b, "discard_packets_sent", gpon_get_ani_stats_discard_packets_sent());
        blobmsg_add_u64(&b, "discard_packets_received", gpon_get_ani_stats_discard_packets_received());
        blobmsg_add_u64(&b, "multicast_packets_sent", gpon_get_onu_ani_stats("us_mcast"));
        blobmsg_add_u64(&b, "multicast_packets_received", gpon_get_onu_ani_stats("ds_mcast"));
        blobmsg_add_u64(&b, "broadcast_packets_sent", gpon_get_onu_ani_stats("us_bcast"));
        blobmsg_add_u64(&b, "broadcast_packets_received", gpon_get_onu_ani_stats("ds_bcast"));
        blobmsg_add_u64(&b, "unknown_proto_packets_received", 0); /* always 0 */
#endif
    }

    else if(str_equal(path, ONU_ACTIVATION, name_len)) {
        SAH_TRACE_INFO("path='%s' => ONU_ACTIVATION", path);
#ifdef BOARD_HERMOSA
        {
            char ani_script_index[8];
            char onu_nr_char = name[strlen("xpon_onu.")];
            snprintf(ani_script_index, sizeof(ani_script_index), "%c.1", onu_nr_char);
            gpon_hermosa_run_script("getXponOnuAniTcOnuActivationtContent", ani_script_index);
            blobmsg_add_string(&b, "onu_state",    gpon_hermosa_parse_field("ONUState"));
            blobmsg_add_string(&b, "vendor_id",    gpon_hermosa_parse_field("VendorID"));
            blobmsg_add_string(&b, "serial_number", gpon_hermosa_parse_field("SerialNumber"));
            blobmsg_add_u32(&b, "onu_id", (uint32_t)gpon_hermosa_parse_field_int("ONUID"));
        }
#else
        /* Marina / Alder / Miami*/
        {
            char onu_state[3];
            snprintf(onu_state, 3, "O%u", s_onu_state);
            blobmsg_add_string(&b, "onu_state", gpon_get_pm_operationalstate());
            blobmsg_add_string(&b, "vendor_id", gpon_get_ploam_vendorid());
            blobmsg_add_string(&b, "serial_number", gpon_get_ploam_serialnumber());
            blobmsg_add_u32(&b, "onu_id", gpon_get_ploam_onuid());
        }
#endif
    }

    else if(str_equal(path, ANU_TC_PM_PHY, name_len)) 
    {
        SAH_TRACE_INFO("path='%s' => ANU_TC_PM_PHY", path);
#ifdef BOARD_HERMOSA
        {
            char ani_script_index[8];
            char onu_nr_char = name[strlen("xpon_onu.")];
            snprintf(ani_script_index, sizeof(ani_script_index), "%c.1", onu_nr_char);
            gpon_hermosa_run_script("getXponOnuAniTcPmPhyContent", ani_script_index);
            blobmsg_add_u64(&b, "corrected_fec_bytes",         gpon_hermosa_parse_field_int("CorrectedFECBytes"));
            blobmsg_add_u64(&b, "corrected_fec_codewords",     gpon_hermosa_parse_field_int("CorrectedFECCodewords"));
            blobmsg_add_u64(&b, "uncorrectable_fec_codewords", gpon_hermosa_parse_field_int("UncorrectableFECCodewords"));
            blobmsg_add_u64(&b, "total_fec_codewords",         gpon_hermosa_parse_field_int("TotalFECCodewords"));
            blobmsg_add_u64(&b, "psbd_hec_error_count",        gpon_hermosa_parse_field_int("PSBdHECErrorCount"));
            blobmsg_add_u64(&b, "header_hec_error_count",      gpon_hermosa_parse_field_int("HeaderHECErrorCount"));
            blobmsg_add_u64(&b, "unknown_profile",             gpon_hermosa_parse_field_int("UnknownProfile"));
        }
#else
        /* Marina / Alder / Miami*/
        blobmsg_add_u64(&b, "corrected_fec_bytes", gpon_get_onu_ani_tc_pm("*fec_byte_corrected"));
        blobmsg_add_u64(&b, "corrected_fec_codewords", gpon_get_onu_ani_tc_pm("fec_codewd_corrected"));
        blobmsg_add_u64(&b, "uncorrectable_fec_codewords", gpon_get_onu_ani_tc_pm("fec_codewd_uncorrected"));
        blobmsg_add_u64(&b, "total_fec_codewords", gpon_get_onu_ani_tc_pm("fec_codewd"));
        blobmsg_add_u64(&b, "psbd_hec_error_count", gpon_get_onu_ani_tc_pm("psbd_hec_err"));
        blobmsg_add_u64(&b, "header_hec_error_count", gpon_get_onu_ani_tc_pm("fs_hec_err"));
        blobmsg_add_u64(&b, "unknown_profile", gpon_get_onu_ani_tc_pm("unknown_bst_prof"));
#endif
    }
    
    else if(str_equal(path, TC_PM_GEM, name_len)) {
        SAH_TRACE_INFO("path='%s' => TC_PM_GEM", path);
#ifdef BOARD_HERMOSA
        {
            char ani_script_index[8];
            char onu_nr_char = name[strlen("xpon_onu.")];
            snprintf(ani_script_index, sizeof(ani_script_index), "%c.1", onu_nr_char);
            gpon_hermosa_run_script("getXponOnuAniTcPmGemContent", ani_script_index);
            blobmsg_add_u64(&b, "frames_sent",             gpon_hermosa_parse_field_int("FramesSent"));
            blobmsg_add_u64(&b, "frames_received",         gpon_hermosa_parse_field_int("FramesReceived"));
            blobmsg_add_u64(&b, "frame_header_hec_errors", gpon_hermosa_parse_field_int("FrameHeaderHECErrors"));
            blobmsg_add_u64(&b, "key_errors",              gpon_hermosa_parse_field_int("KeyErrors"));
        }
#else
        /* Marina / Alder / Miami*/
        blobmsg_add_u64(&b, "frames_sent", gpon_get_gem_framessent(1,8));
        blobmsg_add_u64(&b, "frames_received", gpon_get_gem_framesreceived(1,8));
        blobmsg_add_u64(&b, "frame_header_hec_errors", gpon_get_frame_header_hec_errors());
        blobmsg_add_u64(&b, "key_errors", gpon_get_xgem_key_errors());
#endif
    }
    
    else if(str_equal(path, TC_PM_PLOAM, name_len)){
        SAH_TRACE_INFO("path='%s' => TC_PM_PLOAM", path);
#ifdef BOARD_HERMOSA
        {
            char ani_script_index[8];
            char onu_nr_char = name[strlen("xpon_onu.")];
            snprintf(ani_script_index, sizeof(ani_script_index), "%c.1", onu_nr_char);
            gpon_hermosa_run_script("getXponOnuAniTcPmPloamContent", ani_script_index);
            blobmsg_add_u64(&b, "mic_errors",               gpon_hermosa_parse_field_int("MICErrors"));
            blobmsg_add_u64(&b, "downstream_message_count", gpon_hermosa_parse_field_int("DownstreamMessageCount"));
            blobmsg_add_u64(&b, "ranging_time",             gpon_hermosa_parse_field_int("RangingTime"));
            blobmsg_add_u64(&b, "upstream_message_count",   gpon_hermosa_parse_field_int("UpstreamMessageCount"));
        }
#else
        /* Marina / Alder / Miami*/
        blobmsg_add_u64(&b, "mic_errors", gpon_get_ploam_micerrors());
        blobmsg_add_u64(&b, "downstream_message_count", gpon_get_ploam_txmesgcount());
        blobmsg_add_u64(&b, "ranging_time", gpon_get_ranging_time());
        blobmsg_add_u64(&b, "upstream_message_count", gpon_get_ploam_rxmesgcount());
#endif
    }
    
    else if(str_equal(path, TC_PM_OMCI, name_len)){
        SAH_TRACE_INFO("path='%s' => TC_PM_OMCI", path);
#ifdef BOARD_HERMOSA
        {
            char ani_script_index[8];
            char onu_nr_char = name[strlen("xpon_onu.")];
            snprintf(ani_script_index, sizeof(ani_script_index), "%c.1", onu_nr_char);
            gpon_hermosa_run_script("getXponOnuAniTcPmOmciContent", ani_script_index);
            blobmsg_add_u64(&b, "baseline_messages_received", gpon_hermosa_parse_field_int("BaselineMessagesReceived"));
            blobmsg_add_u64(&b, "extended_messages_received", gpon_hermosa_parse_field_int("ExtendedMessagesReceived"));
            blobmsg_add_u64(&b, "mic_errors",                 gpon_hermosa_parse_field_int("MICErrors"));
        }
#else
        /* Marina / Alder / Miami*/
        blobmsg_add_u64(&b, "baseline_messages_received", gpon_get_omci_baselinemessagecount());
        blobmsg_add_u64(&b, "extended_messages_received", gpon_get_omci_extendedmessagecount());
        blobmsg_add_u64(&b, "mic_errors", gpon_get_omci_micerrors());
#endif
    }

    else if(str_equal(path, TC_GEM, name_len)){
        SAH_TRACE_INFO("path='%s' => TC_GEM", path);
#ifdef BOARD_HERMOSA
        {
            char ani_script_index[8];
            char onu_nr_char = name[strlen("xpon_onu.")];
            snprintf(ani_script_index, sizeof(ani_script_index), "%c.1", onu_nr_char);
            gpon_hermosa_run_script("getXponOnuAniTcGemPortNumberOfEntries", ani_script_index);
            blobmsg_add_u32(&b, "port_number_of_entries", (uint32_t)gpon_hermosa_parse_field_int("PortNumberOfEntries"));
        }
#else
        /* Marina / Alder / Miami*/
        blobmsg_add_u32(&b, "port_number_of_entries", gpon_get_port_number_of_entries());
#endif
    }

    else if(str_equal(path, GEM_PORT_1, name_len))
    {
        SAH_TRACE_INFO("path='%s' => GEM_PORT_1", path);
#ifdef BOARD_HERMOSA
        { char gem_script_index[16]; char onu_nr_char = name[strlen("xpon_onu.")];
          snprintf(gem_script_index, sizeof(gem_script_index), "%c.1.1", onu_nr_char);
          gpon_hermosa_run_script("getXponOnuAniTcGemPortContent", gem_script_index);
          blobmsg_add_u32(&b, "port_id", (uint32_t)gpon_hermosa_parse_field_int("PortID"));
          blobmsg_add_string(&b, "direction", gpon_hermosa_parse_field("Direction"));
          blobmsg_add_string(&b, "port_type", gpon_hermosa_parse_field("PortType")); }
#else
        blobmsg_add_u32(&b, "port_id", gpon_get_gem_port_id(1));
        blobmsg_add_string(&b, "direction", gpon_get_gem_directions(1));
        blobmsg_add_string(&b, "port_type", gpon_get_gem_traffctype(1));
#endif
    }
    else if(str_equal(path, GEM_PORT_2, name_len))
    {
        SAH_TRACE_INFO("path='%s' => GEM_PORT_2", path);
#ifdef BOARD_HERMOSA
        { char gem_script_index[16]; char onu_nr_char = name[strlen("xpon_onu.")];
          snprintf(gem_script_index, sizeof(gem_script_index), "%c.1.2", onu_nr_char);
          gpon_hermosa_run_script("getXponOnuAniTcGemPortContent", gem_script_index);
          blobmsg_add_u32(&b, "port_id", (uint32_t)gpon_hermosa_parse_field_int("PortID"));
          blobmsg_add_string(&b, "direction", gpon_hermosa_parse_field("Direction"));
          blobmsg_add_string(&b, "port_type", gpon_hermosa_parse_field("PortType")); }
#else
        blobmsg_add_u32(&b, "port_id", gpon_get_gem_port_id(2));
        blobmsg_add_string(&b, "direction", gpon_get_gem_directions(2));
        blobmsg_add_string(&b, "port_type", gpon_get_gem_traffctype(2));
#endif
    }
    else if(str_equal(path, GEM_PORT_3, name_len))
    {
        SAH_TRACE_INFO("path='%s' => GEM_PORT_3", path);
#ifdef BOARD_HERMOSA
        { char gem_script_index[16]; char onu_nr_char = name[strlen("xpon_onu.")];
          snprintf(gem_script_index, sizeof(gem_script_index), "%c.1.3", onu_nr_char);
          gpon_hermosa_run_script("getXponOnuAniTcGemPortContent", gem_script_index);
          blobmsg_add_u32(&b, "port_id", (uint32_t)gpon_hermosa_parse_field_int("PortID"));
          blobmsg_add_string(&b, "direction", gpon_hermosa_parse_field("Direction"));
          blobmsg_add_string(&b, "port_type", gpon_hermosa_parse_field("PortType")); }
#else
        blobmsg_add_u32(&b, "port_id", gpon_get_gem_port_id(3));
        blobmsg_add_string(&b, "direction", gpon_get_gem_directions(3));
        blobmsg_add_string(&b, "port_type", gpon_get_gem_traffctype(3));
#endif
    }
    else if(str_equal(path, GEM_PORT_4, name_len))
    {
        SAH_TRACE_INFO("path='%s' => GEM_PORT_4", path);
#ifdef BOARD_HERMOSA
        { char gem_script_index[16]; char onu_nr_char = name[strlen("xpon_onu.")];
          snprintf(gem_script_index, sizeof(gem_script_index), "%c.1.4", onu_nr_char);
          gpon_hermosa_run_script("getXponOnuAniTcGemPortContent", gem_script_index);
          blobmsg_add_u32(&b, "port_id", (uint32_t)gpon_hermosa_parse_field_int("PortID"));
          blobmsg_add_string(&b, "direction", gpon_hermosa_parse_field("Direction"));
          blobmsg_add_string(&b, "port_type", gpon_hermosa_parse_field("PortType")); }
#else
        blobmsg_add_u32(&b, "port_id", gpon_get_gem_port_id(4));
        blobmsg_add_string(&b, "direction", gpon_get_gem_directions(4));
        blobmsg_add_string(&b, "port_type", gpon_get_gem_traffctype(4));
#endif
    }
    else if(str_equal(path, GEM_PORT_5, name_len))
    {
        SAH_TRACE_INFO("path='%s' => GEM_PORT_5", path);
#ifdef BOARD_HERMOSA
        { char gem_script_index[16]; char onu_nr_char = name[strlen("xpon_onu.")];
          snprintf(gem_script_index, sizeof(gem_script_index), "%c.1.5", onu_nr_char);
          gpon_hermosa_run_script("getXponOnuAniTcGemPortContent", gem_script_index);
          blobmsg_add_u32(&b, "port_id", (uint32_t)gpon_hermosa_parse_field_int("PortID"));
          blobmsg_add_string(&b, "direction", gpon_hermosa_parse_field("Direction"));
          blobmsg_add_string(&b, "port_type", gpon_hermosa_parse_field("PortType")); }
#else
        blobmsg_add_u32(&b, "port_id", gpon_get_gem_port_id(5));
        blobmsg_add_string(&b, "direction", gpon_get_gem_directions(5));
        blobmsg_add_string(&b, "port_type", gpon_get_gem_traffctype(5));
#endif
    }
    else if(str_equal(path, GEM_PORT_6, name_len))
    {
        SAH_TRACE_INFO("path='%s' => GEM_PORT_6", path);
#ifdef BOARD_HERMOSA
        { char gem_script_index[16]; char onu_nr_char = name[strlen("xpon_onu.")];
          snprintf(gem_script_index, sizeof(gem_script_index), "%c.1.6", onu_nr_char);
          gpon_hermosa_run_script("getXponOnuAniTcGemPortContent", gem_script_index);
          blobmsg_add_u32(&b, "port_id", (uint32_t)gpon_hermosa_parse_field_int("PortID"));
          blobmsg_add_string(&b, "direction", gpon_hermosa_parse_field("Direction"));
          blobmsg_add_string(&b, "port_type", gpon_hermosa_parse_field("PortType")); }
#else
        blobmsg_add_u32(&b, "port_id", gpon_get_gem_port_id(6));
        blobmsg_add_string(&b, "direction", gpon_get_gem_directions(6));
        blobmsg_add_string(&b, "port_type", gpon_get_gem_traffctype(6));
#endif
    }
    else if(str_equal(path, GEM_PORT_7, name_len))
    {
        SAH_TRACE_INFO("path='%s' => GEM_PORT_7", path);
#ifdef BOARD_HERMOSA
        { char gem_script_index[16]; char onu_nr_char = name[strlen("xpon_onu.")];
          snprintf(gem_script_index, sizeof(gem_script_index), "%c.1.7", onu_nr_char);
          gpon_hermosa_run_script("getXponOnuAniTcGemPortContent", gem_script_index);
          blobmsg_add_u32(&b, "port_id", (uint32_t)gpon_hermosa_parse_field_int("PortID"));
          blobmsg_add_string(&b, "direction", gpon_hermosa_parse_field("Direction"));
          blobmsg_add_string(&b, "port_type", gpon_hermosa_parse_field("PortType")); }
#else
        blobmsg_add_u32(&b, "port_id", gpon_get_gem_port_id(7));
        blobmsg_add_string(&b, "direction", gpon_get_gem_directions(7));
        blobmsg_add_string(&b, "port_type", gpon_get_gem_traffctype(7));
#endif
    }
    else if(str_equal(path, GEM_PORT_8, name_len))
    {
        SAH_TRACE_INFO("path='%s' => GEM_PORT_8", path);
#ifdef BOARD_HERMOSA
        { char gem_script_index[16]; char onu_nr_char = name[strlen("xpon_onu.")];
          snprintf(gem_script_index, sizeof(gem_script_index), "%c.1.8", onu_nr_char);
          gpon_hermosa_run_script("getXponOnuAniTcGemPortContent", gem_script_index);
          blobmsg_add_u32(&b, "port_id", (uint32_t)gpon_hermosa_parse_field_int("PortID"));
          blobmsg_add_string(&b, "direction", gpon_hermosa_parse_field("Direction"));
          blobmsg_add_string(&b, "port_type", gpon_hermosa_parse_field("PortType")); }
#else
        blobmsg_add_u32(&b, "port_id", gpon_get_gem_port_id(8));
        blobmsg_add_string(&b, "direction", gpon_get_gem_directions(8));
        blobmsg_add_string(&b, "port_type", gpon_get_gem_traffctype(8));
#endif
    }

    else if(str_equal(path, PM_GEM_PORT_1, name_len)) 
    {
       SAH_TRACE_INFO("path='%s' => PM_GEM_PORT_1", path);
#ifdef BOARD_HERMOSA
       { char gem_script_index[16]; char onu_nr_char = name[strlen("xpon_onu.")];
         snprintf(gem_script_index, sizeof(gem_script_index), "%c.1.1", onu_nr_char);
         gpon_hermosa_run_script("getXponOnuAniTcGemPortPmContent", gem_script_index);
         blobmsg_add_u64(&b, "frames_sent",     gpon_hermosa_parse_field_int("FramesSent"));
         blobmsg_add_u64(&b, "frames_received", gpon_hermosa_parse_field_int("FramesReceived")); }
#else
       blobmsg_add_u64(&b, "frames_sent", gpon_get_gem_transmittedframes(1));
       blobmsg_add_u64(&b, "frames_received", gpon_get_gem_receivedframes(1));
#endif
    } 

    else if(str_equal(path, PM_GEM_PORT_2, name_len)) 
    {
       SAH_TRACE_INFO("path='%s' => PM_GEM_PORT_2", path);
#ifdef BOARD_HERMOSA
       { char gem_script_index[16]; char onu_nr_char = name[strlen("xpon_onu.")];
         snprintf(gem_script_index, sizeof(gem_script_index), "%c.1.2", onu_nr_char);
         gpon_hermosa_run_script("getXponOnuAniTcGemPortPmContent", gem_script_index);
         blobmsg_add_u64(&b, "frames_sent",     gpon_hermosa_parse_field_int("FramesSent"));
         blobmsg_add_u64(&b, "frames_received", gpon_hermosa_parse_field_int("FramesReceived")); }
#else
       blobmsg_add_u64(&b, "frames_sent", gpon_get_gem_transmittedframes(2));
       blobmsg_add_u64(&b, "frames_received", gpon_get_gem_receivedframes(2));
#endif
    }

    else if(str_equal(path, PM_GEM_PORT_3, name_len)) 
    {
       SAH_TRACE_INFO("path='%s' => PM_GEM_PORT_3", path);
#ifdef BOARD_HERMOSA
       { char gem_script_index[16]; char onu_nr_char = name[strlen("xpon_onu.")];
         snprintf(gem_script_index, sizeof(gem_script_index), "%c.1.3", onu_nr_char);
         gpon_hermosa_run_script("getXponOnuAniTcGemPortPmContent", gem_script_index);
         blobmsg_add_u64(&b, "frames_sent",     gpon_hermosa_parse_field_int("FramesSent"));
         blobmsg_add_u64(&b, "frames_received", gpon_hermosa_parse_field_int("FramesReceived")); }
#else
       blobmsg_add_u64(&b, "frames_sent", gpon_get_gem_transmittedframes(3));
       blobmsg_add_u64(&b, "frames_received", gpon_get_gem_receivedframes(3));
#endif
    }

    else if(str_equal(path, PM_GEM_PORT_4, name_len)) 
    {
       SAH_TRACE_INFO("path='%s' => PM_GEM_PORT_4", path);
#ifdef BOARD_HERMOSA
       { char gem_script_index[16]; char onu_nr_char = name[strlen("xpon_onu.")];
         snprintf(gem_script_index, sizeof(gem_script_index), "%c.1.4", onu_nr_char);
         gpon_hermosa_run_script("getXponOnuAniTcGemPortPmContent", gem_script_index);
         blobmsg_add_u64(&b, "frames_sent",     gpon_hermosa_parse_field_int("FramesSent"));
         blobmsg_add_u64(&b, "frames_received", gpon_hermosa_parse_field_int("FramesReceived")); }
#else
       blobmsg_add_u64(&b, "frames_sent", gpon_get_gem_transmittedframes(4));
       blobmsg_add_u64(&b, "frames_received", gpon_get_gem_receivedframes(4));
#endif
    }

    else if(str_equal(path, PM_GEM_PORT_5, name_len)) 
    {
       SAH_TRACE_INFO("path='%s' => PM_GEM_PORT_5", path);
#ifdef BOARD_HERMOSA
       { char gem_script_index[16]; char onu_nr_char = name[strlen("xpon_onu.")];
         snprintf(gem_script_index, sizeof(gem_script_index), "%c.1.5", onu_nr_char);
         gpon_hermosa_run_script("getXponOnuAniTcGemPortPmContent", gem_script_index);
         blobmsg_add_u64(&b, "frames_sent",     gpon_hermosa_parse_field_int("FramesSent"));
         blobmsg_add_u64(&b, "frames_received", gpon_hermosa_parse_field_int("FramesReceived")); }
#else
       blobmsg_add_u64(&b, "frames_sent", gpon_get_gem_transmittedframes(5));
       blobmsg_add_u64(&b, "frames_received", gpon_get_gem_receivedframes(5));
#endif
    }

    else if(str_equal(path, PM_GEM_PORT_6, name_len)) 
    {
       SAH_TRACE_INFO("path='%s' => PM_GEM_PORT_6", path);
#ifdef BOARD_HERMOSA
       { char gem_script_index[16]; char onu_nr_char = name[strlen("xpon_onu.")];
         snprintf(gem_script_index, sizeof(gem_script_index), "%c.1.6", onu_nr_char);
         gpon_hermosa_run_script("getXponOnuAniTcGemPortPmContent", gem_script_index);
         blobmsg_add_u64(&b, "frames_sent",     gpon_hermosa_parse_field_int("FramesSent"));
         blobmsg_add_u64(&b, "frames_received", gpon_hermosa_parse_field_int("FramesReceived")); }
#else
       blobmsg_add_u64(&b, "frames_sent", gpon_get_gem_transmittedframes(6));
       blobmsg_add_u64(&b, "frames_received", gpon_get_gem_receivedframes(6));
#endif
    }

    else if(str_equal(path, PM_GEM_PORT_7, name_len)) 
    {
       SAH_TRACE_INFO("path='%s' => PM_GEM_PORT_7", path);
#ifdef BOARD_HERMOSA
       { char gem_script_index[16]; char onu_nr_char = name[strlen("xpon_onu.")];
         snprintf(gem_script_index, sizeof(gem_script_index), "%c.1.7", onu_nr_char);
         gpon_hermosa_run_script("getXponOnuAniTcGemPortPmContent", gem_script_index);
         blobmsg_add_u64(&b, "frames_sent",     gpon_hermosa_parse_field_int("FramesSent"));
         blobmsg_add_u64(&b, "frames_received", gpon_hermosa_parse_field_int("FramesReceived")); }
#else
       blobmsg_add_u64(&b, "frames_sent", gpon_get_gem_transmittedframes(7));
       blobmsg_add_u64(&b, "frames_received", gpon_get_gem_receivedframes(7));
#endif
    }

    else if(str_equal(path, PM_GEM_PORT_8, name_len)) 
    {
       SAH_TRACE_INFO("path='%s' => PM_GEM_PORT_8", path);
#ifdef BOARD_HERMOSA
       { char gem_script_index[16]; char onu_nr_char = name[strlen("xpon_onu.")];
         snprintf(gem_script_index, sizeof(gem_script_index), "%c.1.8", onu_nr_char);
         gpon_hermosa_run_script("getXponOnuAniTcGemPortPmContent", gem_script_index);
         blobmsg_add_u64(&b, "frames_sent",     gpon_hermosa_parse_field_int("FramesSent"));
         blobmsg_add_u64(&b, "frames_received", gpon_hermosa_parse_field_int("FramesReceived")); }
#else
       blobmsg_add_u64(&b, "frames_sent", gpon_get_gem_transmittedframes(8));
       blobmsg_add_u64(&b, "frames_received", gpon_get_gem_receivedframes(8));
#endif
    }

    else if(str_equal(path, PERF_THRESHOLDS, name_len)) 
    {
        SAH_TRACE_INFO("path='%s' => PERF_THRESHOLDS", path);
#ifdef BOARD_HERMOSA
        {
            char ani_script_index[8];
            char onu_nr_char = name[strlen("xpon_onu.")];
            snprintf(ani_script_index, sizeof(ani_script_index), "%c.1", onu_nr_char);
            gpon_hermosa_run_script("getXponOnuAniTcPerformanceThresholdsContent", ani_script_index);
            blobmsg_add_u32(&b, "signal_fail",    (uint32_t)gpon_hermosa_parse_field_int("SignalFail"));
            blobmsg_add_u32(&b, "signal_degrade", (uint32_t)gpon_hermosa_parse_field_int("SignalDegrade"));
        }
#else
        /* Marina / Alder / Miami*/
        blobmsg_add_u32(&b, "signal_fail", gpon_get_tc_perf_signal_fail());
        blobmsg_add_u32(&b, "signal_degrade", gpon_get_tc_perf_signal_degrade());
#endif
    } 
    
    else if(str_equal(path, ALARMS, name_len))
    {
        SAH_TRACE_INFO("path='%s' => ALARMS", path);
#ifdef BOARD_HERMOSA
        {
            char ani_script_index[8];
            char onu_nr_char = name[strlen("xpon_onu.")];
            snprintf(ani_script_index, sizeof(ani_script_index), "%c.1", onu_nr_char);
            gpon_hermosa_run_script("getXponOnuAniTcAlarmContent", ani_script_index);
            blobmsg_add_u8(&b, "los",   (uint8_t)gpon_hermosa_parse_field_int("LOS"));
            blobmsg_add_u8(&b, "lof",   (uint8_t)gpon_hermosa_parse_field_int("LOF"));
            blobmsg_add_u8(&b, "sf",    (uint8_t)gpon_hermosa_parse_field_int("SF"));
            blobmsg_add_u8(&b, "sd",    (uint8_t)gpon_hermosa_parse_field_int("SD"));
            blobmsg_add_u8(&b, "lcdg",  (uint8_t)gpon_hermosa_parse_field_int("LCDG"));
            blobmsg_add_u8(&b, "tf",    (uint8_t)gpon_hermosa_parse_field_int("TF"));
            blobmsg_add_u8(&b, "suf",   (uint8_t)gpon_hermosa_parse_field_int("SUF"));
            blobmsg_add_u8(&b, "mem",   (uint8_t)gpon_hermosa_parse_field_int("MEM"));
            blobmsg_add_u8(&b, "dact",  (uint8_t)gpon_hermosa_parse_field_int("DACT"));
            blobmsg_add_u8(&b, "dis",   (uint8_t)gpon_hermosa_parse_field_int("DIS"));
            blobmsg_add_u8(&b, "mis",   (uint8_t)gpon_hermosa_parse_field_int("MIS"));
            blobmsg_add_u8(&b, "pee",   (uint8_t)gpon_hermosa_parse_field_int("PEE"));
            blobmsg_add_u8(&b, "rdi",   (uint8_t)gpon_hermosa_parse_field_int("RDI"));
            blobmsg_add_u8(&b, "lods",  (uint8_t)gpon_hermosa_parse_field_int("LODS"));
            blobmsg_add_u8(&b, "rogue", (uint8_t)gpon_hermosa_parse_field_int("ROGUE"));
        }
#else
        /* Marina / Alder / Miami*/
        blobmsg_add_u8(&b, "los", gpon_get_pm_alarm_los());
        blobmsg_add_u8(&b, "lof", gpon_get_pm_alarm_lof());
        blobmsg_add_u8(&b, "sf", gpon_get_pm_alarm_sf());
        blobmsg_add_u8(&b, "sd", gpon_get_pm_alarm_sd());
        blobmsg_add_u8(&b, "lcdg", gpon_get_pm_alarm_lcdg());
        blobmsg_add_u8(&b, "tf", gpon_get_pm_alarm_tf());
        blobmsg_add_u8(&b, "suf", gpon_get_pm_alarm_suf());
        blobmsg_add_u8(&b, "mem", gpon_get_pm_alarm_mem());
        blobmsg_add_u8(&b, "dact", gpon_get_pm_alarm_dact());
        blobmsg_add_u8(&b, "dis", gpon_get_pm_alarm_dis());
        blobmsg_add_u8(&b, "mis", 0);
        blobmsg_add_u8(&b, "pee", 0);
        blobmsg_add_u8(&b, "rdi", 0);
        blobmsg_add_u8(&b, "lods", gpon_get_pm_alarm_lods());
        blobmsg_add_u8(&b, "rogue", gpon_get_pm_alarm_rogue());
#endif
    } 
    
    else if(str_equal(path, TRANSCEIVER_1, name_len)) 
    {
        SAH_TRACE_INFO("path='%s' => TRANSCEIVER_1", path);
#ifdef BOARD_HERMOSA
        {
            /* Hermosa: INDEX format "<onu_nr>.1.1", e.g. "1.1.1" */
            char xcvr_script_index[16];
            char onu_nr_char = name[strlen("xpon_onu.")];
            snprintf(xcvr_script_index, sizeof(xcvr_script_index), "%c.1.1", onu_nr_char);
            gpon_hermosa_run_script("getXponOnuAniTransceiverContent", xcvr_script_index);
            blobmsg_add_u32(&b, "id", 0);
            blobmsg_add_u32(&b, "identifier", (uint32_t)gpon_hermosa_parse_field_int("Identifier"));
            blobmsg_add_string(&b, "vendor_name", gpon_hermosa_parse_field("ModuleVendor"));
            blobmsg_add_string(&b, "vendor_part_number", "");
            blobmsg_add_string(&b, "vendor_revision", "");
            blobmsg_add_string(&b, "pon_mode", gpon_hermosa_parse_field("PONMode"));
            blobmsg_add_string(&b, "connector", gpon_hermosa_parse_field("Connector"));
            blobmsg_add_u64(&b, "nominal_bit_rate_downstream", gpon_hermosa_parse_field_int("NominalBitRateDownstream"));
            blobmsg_add_u64(&b, "nominal_bit_rate_upstream",   gpon_hermosa_parse_field_int("NominalBitRateUpstream"));
            blobmsg_add_u32(&b, "rx_power",     (uint32_t)gpon_hermosa_parse_field_int("RxPower"));
            blobmsg_add_u32(&b, "tx_power",     (uint32_t)gpon_hermosa_parse_field_int("TxPower"));
            blobmsg_add_u32(&b, "voltage",      (uint32_t)gpon_hermosa_parse_field_int("Voltage"));
            blobmsg_add_u32(&b, "bias",         (uint32_t)gpon_hermosa_parse_field_int("Bias"));
            blobmsg_add_u32(&b, "temperature",  (uint32_t)gpon_hermosa_parse_field_int("Temperature"));
        }
#else
        /* Marina / Alder / Miami*/
        {
            char vendor_rev[64];
            snprintf(vendor_rev, 64, "Version_%d", s_transceiver_1_vendor_rev);
            blobmsg_add_u32(&b, "id", 0);
            blobmsg_add_u32(&b, "identifier", 2);
            blobmsg_add_string(&b, "vendor_name", gpon_get_PM_vendorname());
            blobmsg_add_string(&b, "vendor_part_number", gpon_get_PM_vendorpartnumber());
            blobmsg_add_string(&b, "vendor_revision", gpon_get_pm_modulefirmwareversion());
            blobmsg_add_string(&b, "pon_mode", gpon_get_PM_ponmode());
            blobmsg_add_string(&b, "connector", gpon_get_PM_connector());
            blobmsg_add_u64(&b, "nominal_bit_rate_downstream", gpon_get_nominalbitrate());
            blobmsg_add_u64(&b, "nominal_bit_rate_upstream", gpon_get_nominalbitrate());
            s_rx_power=gpon_get_rxp_signallevel();
            blobmsg_add_u32(&b, "rx_power", s_rx_power);
            s_tx_power=gpon_get_txp_signallevel();
            blobmsg_add_u32(&b, "tx_power", s_tx_power);
            s_voltage=gpon_get_voltage();
            blobmsg_add_u32(&b, "voltage", s_voltage);
            s_bias=gpon_get_bias();
            blobmsg_add_u32(&b, "bias", s_bias);
            s_temperature=gpon_get_temperature();
            blobmsg_add_u32(&b, "temperature", s_temperature);
        }
#endif
    } 
    
    else if(str_equal(path, TRANSCEIVER_2, name_len)) 
    {
        SAH_TRACE_INFO("path='%s' => TRANSCEIVER_2", path);
        blobmsg_add_u32(&b, "id", 1);  
        blobmsg_add_u32(&b, "identifier", 25);  
        blobmsg_add_string(&b, "vendor_name", "Qualcomm");  
        blobmsg_add_string(&b, "pon_mode", "XGS-PON");  
    } 
    
    else 
    {
        SAH_TRACE_ERROR("Unknown object: %s", name);
    }
}


static void set_enable(const char* const obj_name, bool enable) {
    printf("--------------- inside set_enable function -----------------\n");
    char path[128];
    snprintf(path, 128, "%s", obj_name);
    const int index_of_onu_nr = strlen("xpon_onu.");
    path[index_of_onu_nr] = 'x';
    const size_t path_len = strlen(obj_name);

    if(str_equal(path, XPON_ONU, path_len))
    {
        if(s_onu_enabled != enable)
        {
            SAH_TRACE_INFO("s_onu_enabled: %d -> %d", s_onu_enabled, enable);
            s_onu_enabled = enable;
            blobmsg_add_u8(&b, "enable", s_onu_enabled);
            gpon_set_xpon_onu_enable(enable);
        }
        blobmsg_add_u8(&b, "enable", s_onu_enabled);
    } 

    else if(str_equal(path, ANI_1, path_len)) 
    {
        if(s_ani_one_enabled != enable) 
        {
            SAH_TRACE_INFO("s_ani_one_enabled: %d -> %d", s_ani_one_enabled, enable);
            s_ani_one_enabled = enable;
            blobmsg_add_u8(&b, "enable", s_ani_one_enabled);
            gpon_set_xpon_onu_ani_enable(enable);
        }
        blobmsg_add_u8(&b, "enable", s_ani_one_enabled);
    }

    else 
    {
        SAH_TRACE_ERROR("Unknown object: '%s'", path);
    }
}

static void data_model_for_get_params_method(const char* const obj_name,
                                                 const char* const param_name) {
    char path[128];
    snprintf(path, 128, "%s", obj_name);
    const int index_of_onu_nr = strlen("xpon_onu.");
    path[index_of_onu_nr] = 'x';
    const size_t path_len = strlen(path);

    if(str_equal(path, TRANSCEIVER_1, path_len) ||
       str_equal(path, TRANSCEIVER_2, path_len)) {

        const size_t len = strlen(param_name);

        if(str_equal(param_name, "rx_power", len)) {
            blobmsg_add_u32(&b, "rx_power", s_rx_power);
            ++s_rx_power;
        } else if(str_equal(param_name, "tx_power", len)) {
            blobmsg_add_u32(&b, "tx_power", s_tx_power);
            ++s_tx_power;
        } else if(str_equal(param_name, "voltage", len)) {
            blobmsg_add_u32(&b, "voltage", s_voltage);
            ++s_voltage;
        } else if(str_equal(param_name, "bias", len)) {
            blobmsg_add_u32(&b, "bias", s_bias);
            ++s_bias;
        } else if(str_equal(param_name, "temperature", len)) {
            blobmsg_add_u32(&b, "temperature", s_temperature);
            ++s_temperature;
        } else {
            SAH_TRACE_ERROR("object: %s: unknown param_name: %s", obj_name, param_name);
        }
    } 

    else {
        SAH_TRACE_ERROR("Unknown object: %s", obj_name);
    }
}


static void method_cb(struct uloop_timeout* t) {
    request_t* req = container_of(t, request_t, timeout);
    when_null(req, exit);
    when_null_trace(req->method_name, exit, ERROR, "method_name is NULL");
    const char* const method = req->method_name; /* alias */
    const size_t method_len = strlen(method);

    blob_buf_init(&b, 0);
    if(str_equal(method, METHOD_GET, method_len)) {
        data_model_for_get_method(req->obj_name);
    } else if(str_equal(method, METHOD_ENABLE, method_len)) {
        set_enable(req->obj_name, true);
    } else if(str_equal(method, METHOD_DISABLE, method_len)) {
        set_enable(req->obj_name, false);
    } else if(str_equal(method, METHOD_GET_PARAMS, method_len)) {
        data_model_for_get_params_method(req->obj_name, req->param_name);
    } else {
        SAH_TRACE_ERROR("Unknown method: '%s'", method);
    }

    if(ubus_send_reply(s_ctx, &req->req, b.head)) {
        SAH_TRACE_ERROR("Failed to send msg");
    }

    ubus_complete_deferred_request(s_ctx, &req->req, 0);

exit:
    request_delete(req);
    return;
}

static int common_method_handler(struct ubus_context* ctx, struct ubus_object* obj,
                                 struct ubus_request_data* req, const char* method,
                                 struct blob_attr* msg) {
    int rc = UBUS_STATUS_UNKNOWN_ERROR;

    when_null_trace(obj, exit, ERROR, "obj is NULL");
    when_null_trace(obj->name, exit, ERROR, "obj->name is NULL");

    SAH_TRACE_INFO("%s.%s()", obj->name, method);

    request_t* mreq = (request_t*) calloc(1, sizeof(request_t));
    when_null_trace(mreq, exit, ERROR, "Failed to allocate mem for request_t");

    ubus_defer_request(ctx, req, &mreq->req);

    mreq->obj_name = strdup(obj->name);
    mreq->method_name = strdup(method);
    mreq->timeout.cb = method_cb;

    struct blob_attr* tb[MAX_PARAMS];
    if(str_equal(method, METHOD_GET_PARAMS, strlen(method))) {
        blobmsg_parse(GET_PARAMS_POLICY, ARRAY_SIZE(GET_PARAMS_POLICY), tb, blob_data(msg), blob_len(msg));
        if(tb[PARAM_NAME] != NULL) {
            char* param_name = blobmsg_data(tb[PARAM_NAME]);
            if(param_name) {
                SAH_TRACE_DEBUG("param: %s", param_name);
                mreq->param_name = strdup(param_name);
            } else {
                SAH_TRACE_ERROR("%s.%s(): failed to get param name(s)", obj->name, method);
                request_delete(mreq);
                goto exit;
            }
        } else {
            SAH_TRACE_ERROR("%s.%s(): failed to get param name(s)", obj->name, method);
            request_delete(mreq);
            goto exit;
        }
    }

    uloop_timeout_set(&mreq->timeout, 0);

    rc = 0;

exit:
    return rc;
}

static void delete_object_wrapper(object_wrapper_t* obj) {
    if(obj->ubus_obj.type) {
        free(obj->ubus_obj.type);
    }
    if(obj->name) {
        free(obj->name);
    }
    free(obj);
}


static object_wrapper_t* create_object_wrapper(obj_id_t id, const char* path) {
    object_wrapper_t* obj = (object_wrapper_t*) calloc(1, sizeof(object_wrapper_t));
    when_null_trace(obj, exit, ERROR, "Failed to allocate mem");

    obj->ubus_obj.type = (struct ubus_object_type*) calloc(1, sizeof(struct ubus_object_type));
    if(NULL == obj->ubus_obj.type) {
        SAH_TRACE_ERROR("Failed to allocate mem for ubus_object_type");
        free(obj);
        return NULL;
    }

    obj->name = strdup(path);

    obj->ubus_obj.name = obj->name;
    obj->ubus_obj.methods = OBJECT_INFO[id].methods;
    obj->ubus_obj.n_methods = OBJECT_INFO[id].n_methods;

    obj->ubus_obj.type->name = obj->name;
    obj->ubus_obj.type->methods = OBJECT_INFO[id].methods;
    obj->ubus_obj.type->n_methods = OBJECT_INFO[id].n_methods;

exit:
    return obj;
}

static int register_object(obj_id_t id) {
    int rc = 1;

    if(id >= n_static_objects) {
        SAH_TRACE_ERROR("id=%d >= n_static_objects=%d", id, n_static_objects);
        goto exit;
    }

    char path[128];
    const int index = strlen("xpon_onu."); /* index of 'x' char */

    snprintf(path, 128, "%s", OBJECT_INFO[id].name);
    /* Replace 'x' char with ONU nr: '1' or '2'. */
    path[index] = DIGITS[s_onu_nr];

    object_wrapper_t* obj = create_object_wrapper(id, path);
    when_null_trace(obj, exit, ERROR,
                    "Failed to create object for %s", path);

    if(ubus_add_object(s_ctx, &obj->ubus_obj) != UBUS_STATUS_OK) {
        SAH_TRACE_ERROR("Failed to add '%s' to ubus", path);
        delete_object_wrapper(obj);
        goto exit;
    }
    s_objects[id] = obj;

    rc = 0;
exit:
    return rc;
}

static void unregister_object(obj_id_t id) {
    if(id >= n_static_objects) {
        SAH_TRACE_ERROR("id=%d >= n_static_objects=%d", id, n_static_objects);
        return;
    }
    if(s_objects[id] == NULL) {
        SAH_TRACE_DEBUG("Object with id=%d does not exist", id);
        return;
    }

    object_wrapper_t* obj = s_objects[id];

    if(ubus_remove_object(s_ctx, &obj->ubus_obj)) {
        SAH_TRACE_ERROR("Failed to remove '%s' from ubus", obj->name);
    }
    delete_object_wrapper(obj);
    s_objects[id] = NULL;
}

// Helper function to extract UNI index from object name
static int extract_uni_index_from_name(const char* name) {
    // Expected format: "xpon_onu.1.ethernet_uni.N" or "xpon_onu.1.ethernet_uni.N.stats"
    const char* uni_pos = strstr(name, "ethernet_uni.");
    if (!uni_pos) {
        return -1;
    }
    
    uni_pos += strlen("ethernet_uni.");
    int index = 0;
    if (sscanf(uni_pos, "%d", &index) == 1) {
        return index;
    }
    
    return -1;
}

// Helper function to check if object name is ethernet UNI
static int is_ethernet_uni_object(const char* name) {
    return strstr(name, "ethernet_uni.") != NULL ? 1 : 0;
}

// Helper function to check if object name is ethernet UNI stats
static int is_ethernet_uni_stats_object(const char* name) {
    return (strstr(name, "ethernet_uni.") != NULL && strstr(name, ".stats") != NULL) ? 1 : 0;
}

// Create dynamic ethernet UNI object wrapper
static object_wrapper_t* create_eth_uni_object_wrapper(const char* path, int is_stats) {
    object_wrapper_t* obj = (object_wrapper_t*) calloc(1, sizeof(object_wrapper_t));
    when_null_trace(obj, exit, ERROR, "Failed to allocate mem");

    obj->ubus_obj.type = (struct ubus_object_type*) calloc(1, sizeof(struct ubus_object_type));
    if(NULL == obj->ubus_obj.type) {
        SAH_TRACE_ERROR("Failed to allocate mem for ubus_object_type");
        free(obj);
        return NULL;
    }

    obj->name = strdup(path);

    obj->ubus_obj.name = obj->name;
    obj->ubus_obj.methods = METHODS_GET_ONLY;
    obj->ubus_obj.n_methods = ARRAY_SIZE(METHODS_GET_ONLY);

    obj->ubus_obj.type->name = obj->name;
    obj->ubus_obj.type->methods = METHODS_GET_ONLY;
    obj->ubus_obj.type->n_methods = ARRAY_SIZE(METHODS_GET_ONLY);

exit:
    return obj;
}

// Initialize dynamic ethernet UNI objects
static int dm_init_ethernet_uni_objects(int onu_nr) {
    int eth_uni_count = gpon_get_eth_uni_number_of_entries();
    
    if (eth_uni_count <= 0) {
        SAH_TRACE_INFO("No ethernet UNI entries to register");
        return 0;
    }
    
    SAH_TRACE_INFO("Initializing %d ethernet UNI objects", eth_uni_count);
    
    // Clean up any existing allocations
    if (s_eth_uni_objects || s_eth_uni_stats_objects || eth_uni_names || eth_uni_stats_names) {
        SAH_TRACE_INFO("Cleaning up existing ethernet UNI objects");
    }
    
    max_eth_uni_count = eth_uni_count;
    
    // Allocate arrays for object pointers
    s_eth_uni_objects = (object_wrapper_t**) calloc(eth_uni_count, sizeof(object_wrapper_t*));
    s_eth_uni_stats_objects = (object_wrapper_t**) calloc(eth_uni_count, sizeof(object_wrapper_t*));
    eth_uni_names = (char**) calloc(eth_uni_count, sizeof(char*));
    eth_uni_stats_names = (char**) calloc(eth_uni_count, sizeof(char*));
    
    if (!s_eth_uni_objects || !s_eth_uni_stats_objects || !eth_uni_names || !eth_uni_stats_names) {
        SAH_TRACE_ERROR("Failed to allocate memory for ethernet UNI objects");
        return 1;
    }
    
    const int index = strlen("xpon_onu."); /* index of 'x' char */
    
    // Create and register ethernet UNI objects
    for (int i = 0; i < eth_uni_count; i++) {
        char uni_path[128];
        char stats_path[128];
        
        // Create ethernet UNI object name: "xpon_onu.1.ethernet_uni.N"
        snprintf(uni_path, sizeof(uni_path), "xpon_onu.x.ethernet_uni.%d", i + 1);
        uni_path[index] = DIGITS[onu_nr];
        
        // Create ethernet UNI stats object name: "xpon_onu.1.ethernet_uni.N.stats"
        snprintf(stats_path, sizeof(stats_path), "xpon_onu.x.ethernet_uni.%d.stats", i + 1);
        stats_path[index] = DIGITS[onu_nr];
        
        // Store names for later reference
        eth_uni_names[i] = strdup(uni_path);
        eth_uni_stats_names[i] = strdup(stats_path);
        
        // Create and register ethernet UNI object
        s_eth_uni_objects[i] = create_eth_uni_object_wrapper(uni_path, 0);
        if (!s_eth_uni_objects[i]) {
            SAH_TRACE_ERROR("Failed to create ethernet UNI object %d", i + 1);
            return 1;
        }
        
        if (ubus_add_object(s_ctx, &s_eth_uni_objects[i]->ubus_obj) != UBUS_STATUS_OK) {
            SAH_TRACE_ERROR("Failed to add ethernet UNI object '%s' to ubus", uni_path);
            delete_object_wrapper(s_eth_uni_objects[i]);
            s_eth_uni_objects[i] = NULL;
            return 1;
        }
        
        // Create and register ethernet UNI stats object
        s_eth_uni_stats_objects[i] = create_eth_uni_object_wrapper(stats_path, 1);
        if (!s_eth_uni_stats_objects[i]) {
            SAH_TRACE_ERROR("Failed to create ethernet UNI stats object %d", i + 1);
            return 1;
        }
        
        if (ubus_add_object(s_ctx, &s_eth_uni_stats_objects[i]->ubus_obj) != UBUS_STATUS_OK) {
            SAH_TRACE_ERROR("Failed to add ethernet UNI stats object '%s' to ubus", stats_path);
            delete_object_wrapper(s_eth_uni_stats_objects[i]);
            s_eth_uni_stats_objects[i] = NULL;
            return 1;
        }
        
        SAH_TRACE_INFO("Registered ethernet UNI objects: %s and %s", uni_path, stats_path);
    }
    
    return 0;
}

static int dm_advertise_onu(int onu_nr) {
    int rc = 1;

    const size_t n_digits = ARRAY_SIZE(DIGITS);
    if(onu_nr > (int) n_digits) {
        SAH_TRACE_ERROR("Invalid onu_nr [%d]", onu_nr);
        goto exit;
    }

    size_t i;

    // Register static objects (excluding ethernet UNI which are now dynamic)
    for(i = 0; i < id_transceiver_2; ++i) {
        if(register_object(i) != 0) {
            goto exit;
        }
    }
    
    // Initialize dynamic ethernet UNI objects
    if (dm_init_ethernet_uni_objects(onu_nr) != 0) {
        SAH_TRACE_ERROR("Failed to initialize ethernet UNI objects");
        goto exit;
    }

    rc = 0;

exit:
    return rc;
}

int dm_init(struct ubus_context* ctx, int onu_nr) {
    s_ctx = ctx;
    s_onu_nr = onu_nr;

    return dm_advertise_onu(onu_nr);
}

object_wrapper_t* dm_get_xpon_onu_object(void) {
    return s_objects[id_xpon_onu];
}

int dm_register_transceiver_two(void) {
    if(s_objects[id_transceiver_2] != NULL) {
        SAH_TRACE_ERROR("transceiver.2 already exists");
        return 1;
    }

    return register_object(id_transceiver_2);
}

void dm_unregister_transceiver_two(void) {
    unregister_object(id_transceiver_2);
}

void dm_change_transceiver_one_vendor_rev(void) {
    ++s_transceiver_1_vendor_rev;
}

void dm_change_onu_activation_onu_state(void) {
    ++s_onu_state;
    if(s_onu_state > 9) {
        s_onu_state = 1;
    }
}

void dm_cleanup(void) {
    int i;

    // Clean up static objects
    for(i = 0; i < n_static_objects; ++i) {
        if(s_objects[i]) {
            unregister_object(i);
        }
    }
    
    // Clean up dynamic ethernet UNI objects
    if(s_eth_uni_objects) {
        for(i = 0; i < max_eth_uni_count; ++i) {
            if(s_eth_uni_objects[i]) {
                if(ubus_remove_object(s_ctx, &s_eth_uni_objects[i]->ubus_obj)) {
                    SAH_TRACE_ERROR("Failed to remove ethernet UNI object from ubus");
                }
                delete_object_wrapper(s_eth_uni_objects[i]);
            }
        }
        free(s_eth_uni_objects);
        s_eth_uni_objects = NULL;
    }
    
    // Clean up dynamic ethernet UNI stats objects
    if(s_eth_uni_stats_objects) {
        for(i = 0; i < max_eth_uni_count; ++i) {
            if(s_eth_uni_stats_objects[i]) {
                if(ubus_remove_object(s_ctx, &s_eth_uni_stats_objects[i]->ubus_obj)) {
                    SAH_TRACE_ERROR("Failed to remove ethernet UNI stats object from ubus");
                }
                delete_object_wrapper(s_eth_uni_stats_objects[i]);
            }
        }
        free(s_eth_uni_stats_objects);
        s_eth_uni_stats_objects = NULL;
    }
    
    // Clean up dynamic name arrays
    if(eth_uni_names) {
        for(i = 0; i < max_eth_uni_count; ++i) {
            if(eth_uni_names[i]) {
                free(eth_uni_names[i]);
            }
        }
        free(eth_uni_names);
        eth_uni_names = NULL;
    }
    
    if(eth_uni_stats_names) {
        for(i = 0; i < max_eth_uni_count; ++i) {
            if(eth_uni_stats_names[i]) {
                free(eth_uni_stats_names[i]);
            }
        }
        free(eth_uni_stats_names);
        eth_uni_stats_names = NULL;
    }
    
    max_eth_uni_count = 0;
}
