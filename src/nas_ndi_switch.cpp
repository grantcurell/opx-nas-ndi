/*
 * Copyright (c) 2019 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

/*
 * filename: nas_ndi_switch.c
 */

extern "C" {
#include "sai.h"
#include "saiswitch.h"
#include "saiswitchextensions.h"
#include "saiextensions.h"
#include "dell-base-switch-element.h"
}

#include "nas_ndi_switch.h"

#include "std_error_codes.h"
#include "ds_common_types.h"
#include "event_log_types.h"
#include "nas_ndi_int.h"
#include "nas_ndi_utils.h"
#include "nas_ndi_event_logs.h"


#include <unordered_map>
#include <algorithm>
#include <vector>

t_std_error ndi_switch_attr_get(npu_id_t npu,  sai_attribute_t *attr, size_t count) {
    sai_status_t sai_ret = SAI_STATUS_FAILURE;

    if (count==0) return STD_ERR_OK;

    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(npu);
    if (ndi_db_ptr == NULL) {
        return STD_ERR(NPU, PARAM, 0);
    }

    if ((sai_ret = ndi_sai_switch_api_tbl_get(ndi_db_ptr)->get_switch_attribute(ndi_switch_id_get(),
            (uint32_t)count,attr)) != SAI_STATUS_SUCCESS) {
        NDI_LOG_TRACE("NDI-SWITCH", "Error from SAI:%d in get attrs (at least id:%d) for NPU:%d",
                      sai_ret,attr->id, (int)npu);
         return STD_ERR(NPU, CFG, sai_ret);
    }

    return STD_ERR_OK;
}

static t_std_error ndi_switch_attr_set(npu_id_t npu,  const sai_attribute_t *attr) {
    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(npu);
    if (ndi_db_ptr == NULL) {
        return STD_ERR(NPU, PARAM, 0);
    }

    if ((sai_ret = ndi_sai_switch_api_tbl_get(ndi_db_ptr)->set_switch_attribute(ndi_switch_id_get(),attr))
            != SAI_STATUS_SUCCESS) {
        NDI_LOG_TRACE("NDI-SAI", "Error from SAI:%d for attr:%d in default set for NPU:%d",
                      sai_ret,attr->id,npu);
         return STD_ERR(NPU, CFG, sai_ret);
    }
    return STD_ERR_OK;
}


//Expectation is that the left side is the SAI value, while the right side is the NDI type
typedef std::unordered_map<uint32_t,uint32_t> _enum_map;

static bool to_sai_type(_enum_map &ens, sai_attribute_t *param ) {
    auto it = std::find_if(ens.begin(),ens.end(),
            [param](decltype(*ens.begin()) &a) { return a.second == param->value.u32; });
    if (it==ens.end()) return false;
    param->value.u32 = it->first;
    return true;
}

static bool from_sai_type(_enum_map &ens, sai_attribute_t *param ) {
    auto it = ens.find(param->value.u32);
    if (it==ens.end()) return false;
    param->value.u32 = it->second;
    return true;
}

static _enum_map _algo_stoy  = {
    {SAI_HASH_ALGORITHM_XOR, BASE_SWITCH_HASH_ALGORITHM_XOR },
    {SAI_HASH_ALGORITHM_CRC, BASE_SWITCH_HASH_ALGORITHM_CRC },
    {SAI_HASH_ALGORITHM_RANDOM, BASE_SWITCH_HASH_ALGORITHM_RANDOM },
    {SAI_HASH_ALGORITHM_CRC_CCITT, BASE_SWITCH_HASH_ALGORITHM_CRC16CC },
    {SAI_HASH_ALGORITHM_CRC_32LO, BASE_SWITCH_HASH_ALGORITHM_CRC32LSB },
    {SAI_HASH_ALGORITHM_CRC_32HI, BASE_SWITCH_HASH_ALGORITHM_CRC32MSB },
    {SAI_HASH_ALGORITHM_CRC_XOR8, BASE_SWITCH_HASH_ALGORITHM_XOR8 },
    {SAI_HASH_ALGORITHM_CRC_XOR4, BASE_SWITCH_HASH_ALGORITHM_XOR4 },
    {SAI_HASH_ALGORITHM_CRC_XOR2, BASE_SWITCH_HASH_ALGORITHM_XOR2 },
    {SAI_HASH_ALGORITHM_CRC_XOR1, BASE_SWITCH_HASH_ALGORITHM_XOR1 },
};

static bool to_sai_type_hash_algo(sai_attribute_t *param ) {
    return to_sai_type(_algo_stoy,param);
}

static bool from_sai_type_hash_algo(sai_attribute_t *param ) {
    return from_sai_type(_algo_stoy,param);
}

static _enum_map _mode_stoy = {
        {SAI_SWITCH_SWITCHING_MODE_CUT_THROUGH , BASE_SWITCH_SWITCHING_MODE_CUT_THROUGH},
        {SAI_SWITCH_SWITCHING_MODE_STORE_AND_FORWARD, BASE_SWITCH_SWITCHING_MODE_STORE_AND_FORWARD }
};

static bool to_sai_type_switch_mode(sai_attribute_t *param ) {
    return to_sai_type(_mode_stoy,param);
}

static bool from_sai_type_switch_mode(sai_attribute_t *param ) {
    return from_sai_type(_mode_stoy,param);
}

static bool to_sai_type_rate_adjust(sai_attribute_t *param ) {
    param->value.u8 = (uint8_t)param->value.u32;
    return true;
}

static bool from_sai_type_rate_adjust(sai_attribute_t *param ) {
    param->value.u32 = param->value.u8;
    return true;
}

static _enum_map _bst_tracking_mode_stoy = {
        {SAI_SWITCH_BST_TRACKING_MODE_PEAK, BASE_SWITCH_BST_TRACKING_MODE_PEAK},
        {SAI_SWITCH_BST_TRACKING_MODE_CURRENT, BASE_SWITCH_BST_TRACKING_MODE_CURRENT }
};

static bool to_sai_type_bst_tracking_mode(sai_attribute_t *param ) {
    return to_sai_type(_bst_tracking_mode_stoy,param);
}

static bool from_sai_type_bst_tracking_mode(sai_attribute_t *param ) {
    return from_sai_type(_bst_tracking_mode_stoy,param);
}

enum nas_ndi_switch_attr_op_type {
    SW_ATTR_BOOL,
    SW_ATTR_U16,
    SW_ATTR_U32,
    SW_ATTR_S32,
    SW_ATTR_LST,
    SW_ATTR_MAC,
};

struct _sai_op_table {
    nas_ndi_switch_attr_op_type type;
    bool (*to_sai_type)( sai_attribute_t *param );
    bool (*from_sai_type)(sai_attribute_t *param );
    sai_attr_id_t id;
};

static std::unordered_map<uint32_t,_sai_op_table> _attr_to_op = {
    /* BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_DEFAULT_PROFILE
       to
        BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_UFT_MODE_INFO
    are not supported from NDI, the values are returned from NAS itself.
    for the list of attrs these are added here. in future when support
    added for synamic update of these values,corresponding values
    needs to be updated
    */
    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_DEFAULT_PROFILE, {
            SW_ATTR_U32, NULL, NULL, 0 } },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_SUPPORTED_PROFILES, {
            SW_ATTR_U32, NULL, NULL, 0 } },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_SWITCH_PROFILE, {
            SW_ATTR_U32, NULL, NULL, 0 } },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_UFT_MODE, {
            SW_ATTR_U32, NULL, NULL, 0 } },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_UFT_MODE_INFO, {
            SW_ATTR_LST, NULL, NULL, 0 } },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_DEFAULT_MAC_ADDRESS, {
            SW_ATTR_MAC, NULL, NULL, SAI_SWITCH_ATTR_SRC_MAC_ADDRESS } },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_IPV6_EXTENDED_PREFIX_ROUTES, {
            SW_ATTR_U32, NULL, NULL, 0} },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_MAX_IPV6_EXTENDED_PREFIX_ROUTES, {
            SW_ATTR_U32, NULL, NULL, 0} },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_IPV6_EXTENDED_PREFIX_ROUTES_LPM_BLOCK_SIZE, {
            SW_ATTR_U32, NULL, NULL, 0 } },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_LAG_HASH_ALGORITHM, {
            SW_ATTR_U32, to_sai_type_hash_algo, from_sai_type_hash_algo, SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM } },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_LAG_HASH_SEED_VALUE, {
            SW_ATTR_U32, NULL, NULL, SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED } },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_ECMP_HASH_ALGORITHM, {
            SW_ATTR_U32, to_sai_type_hash_algo, from_sai_type_hash_algo, SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM } },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_ECMP_HASH_SEED_VALUE, {
            SW_ATTR_U32, NULL, NULL, SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED } },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_SWITCH_MODE, {
            SW_ATTR_U32, to_sai_type_switch_mode, from_sai_type_switch_mode, SAI_SWITCH_ATTR_SWITCHING_MODE } },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_BRIDGE_TABLE_SIZE, {
            SW_ATTR_U32, NULL,NULL,SAI_SWITCH_ATTR_FDB_TABLE_SIZE}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_MAX_ECMP_ENTRY_PER_GROUP, {
            SW_ATTR_U32, NULL,NULL,SAI_SWITCH_ATTR_ECMP_MEMBERS}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_MAC_AGE_TIMER, {
            SW_ATTR_U32, NULL,NULL,SAI_SWITCH_ATTR_FDB_AGING_TIME}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_TEMPERATURE, {
            SW_ATTR_S32, NULL,NULL,SAI_SWITCH_ATTR_MAX_TEMP}},
    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_ACL_TABLE_MIN_PRIORITY, {
            SW_ATTR_U32, NULL,NULL,SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_ACL_TABLE_MAX_PRIORITY, {
            SW_ATTR_U32, NULL,NULL,SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_ACL_ENTRY_MIN_PRIORITY, {
            SW_ATTR_U32, NULL,NULL,SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_ACL_ENTRY_MAX_PRIORITY, {
            SW_ATTR_U32, NULL,NULL,SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_MAX_MTU, {
            SW_ATTR_U32, NULL,NULL,SAI_SWITCH_ATTR_PORT_MAX_MTU}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_NUM_UNICAST_QUEUES_PER_PORT, {
            SW_ATTR_U32, NULL,NULL, SAI_SWITCH_ATTR_NUMBER_OF_UNICAST_QUEUES}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_NUM_MULTICAST_QUEUES_PER_PORT, {
            SW_ATTR_U32, NULL,NULL, SAI_SWITCH_ATTR_NUMBER_OF_MULTICAST_QUEUES}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_NUM_QUEUES_PER_PORT, {
            SW_ATTR_U32, NULL,NULL, SAI_SWITCH_ATTR_NUMBER_OF_QUEUES}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_NUM_QUEUES_CPU_PORT, {
            SW_ATTR_U32, NULL,NULL, SAI_SWITCH_ATTR_NUMBER_OF_CPU_QUEUES}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_TOTAL_BUFFER_SIZE, {
            SW_ATTR_U32, NULL,NULL,SAI_SWITCH_ATTR_TOTAL_BUFFER_SIZE}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_INGRESS_BUFFER_POOL_NUM, {
            SW_ATTR_U32, NULL,NULL,SAI_SWITCH_ATTR_INGRESS_BUFFER_POOL_NUM}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_EGRESS_BUFFER_POOL_NUM, {
            SW_ATTR_U32, NULL,NULL,SAI_SWITCH_ATTR_EGRESS_BUFFER_POOL_NUM}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_COUNTER_REFRESH_INTERVAL, {
            SW_ATTR_U32, NULL,NULL,SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_ECN_ECT_THRESHOLD_ENABLE, {
            SW_ATTR_U32, NULL,NULL,SAI_SWITCH_ATTR_ECN_ECT_THRESHOLD_ENABLE}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_ECMP_GROUP_SIZE, {
            SW_ATTR_U32, NULL,NULL,SAI_SWITCH_ATTR_NUMBER_OF_ECMP_GROUPS}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_QOS_RATE_ADJUST, {
            SW_ATTR_U32, to_sai_type_rate_adjust,from_sai_type_rate_adjust,SAI_SWITCH_ATTR_EXTENSIONS_QOS_RATE_ADJUST}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_VXLAN_RIOT_ENABLE, {
            SW_ATTR_U32, NULL, NULL, 0} },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_MAX_VXLAN_OVERLAY_RIFS, {
            SW_ATTR_U32, NULL, NULL, 0} },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_MAX_VXLAN_OVERLAY_NEXTHOPS, {
            SW_ATTR_U32, NULL, NULL, 0} },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_RIF_TABLE_SIZE, {
            SW_ATTR_U32, NULL, NULL, 0} },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_L3_NEXTHOP_TABLE_SIZE, {
            SW_ATTR_U32, NULL, NULL, 0} },

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_BST_ENABLE, {
            SW_ATTR_BOOL, NULL,NULL, SAI_SWITCH_ATTR_EXTENSIONS_BST_TRACKING_ENABLE}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_BST_TRACKING_MODE, {
            SW_ATTR_U32, to_sai_type_bst_tracking_mode, from_sai_type_bst_tracking_mode,
            SAI_SWITCH_ATTR_EXTENSIONS_BST_TRACKING_MODE}},

    {BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_DEEP_BUFFER_MODE_ENABLE, {
            SW_ATTR_U32, NULL, NULL, 0 }},
};

extern "C" t_std_error ndi_switch_set_attribute(npu_id_t npu, BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_t attr,
        const nas_ndi_switch_param_t *param) {
    auto it = _attr_to_op.find(attr);
    if (it==_attr_to_op.end()) {
        NDI_LOG_ERROR("NDI-SAI", "Invalid operation type for NDI (%d)",attr);
        return STD_ERR(NPU,FAIL,0);
    }
    sai_attribute_t sai_attr;
    sai_attr.id = it->second.id;
    std::vector<sai_int32_t> tmp_lst;

    switch(it->second.type) {
        case SW_ATTR_S32:
            sai_attr.value.s32 = param->s32;
            break;
        case SW_ATTR_U32:
            sai_attr.value.u32 = param->u32;
            break;
        case SW_ATTR_LST:
            sai_attr.value.s32list.count = param->list.len;
            tmp_lst.resize(param->list.len);
            memcpy(&tmp_lst[0],param->list.vals,param->list.len*sizeof(tmp_lst[0]));
            sai_attr.value.s32list.list = &tmp_lst[0];
            break;
        case SW_ATTR_U16:
            sai_attr.value.u16 = param->u16;
            break;
        case SW_ATTR_MAC:
            memcpy(sai_attr.value.mac, param->mac, sizeof(sai_attr.value.mac));
            break;
        case SW_ATTR_BOOL:
            sai_attr.value.booldata = param->u32;
            break;
    }
    if (it->second.to_sai_type!=NULL) {
        if (!(it->second.to_sai_type)(&sai_attr)) {
            NDI_LOG_TRACE("NDI-SAI", "Values are invalid - can't be converted to SAI types (func:%d)",attr);
            return STD_ERR(NPU,PARAM,0);
        }
    }

    return ndi_switch_attr_set(npu,&sai_attr);
}

extern "C" void ndi_switch_attr_list_get (std::vector<uint32_t> &attrlist)
{
    std::unordered_map<uint32_t,_sai_op_table>::iterator it = _attr_to_op.begin();

    while (it != _attr_to_op.end()) {
        attrlist.push_back(it->first);
        ++it;
    }
    return;
}

extern "C" t_std_error ndi_switch_get_attribute(npu_id_t npu, BASE_SWITCH_SWITCHING_ENTITIES_SWITCHING_ENTITY_t attr,
        nas_ndi_switch_param_t *param) {
    auto it = _attr_to_op.find(attr);
    if (it==_attr_to_op.end()) {
        NDI_LOG_ERROR("NDI-SAI", "Invalid operation type for NDI (%d)",attr);
        return STD_ERR(NPU,FAIL,0);
    }
    sai_attribute_t sai_attr;
    sai_attr.id = it->second.id;

    switch(it->second.type) {
    case SW_ATTR_LST:
        sai_attr.value.s32list.count = param->list.len;
        sai_attr.value.s32list.list = (int32_t*)param->list.vals;
        break;
    default:
        break;
    }

    t_std_error rc = ndi_switch_attr_get(npu,&sai_attr,1);

    if(rc!=STD_ERR_OK) return rc;
    if (it->second.from_sai_type!=NULL) {
        if (!(it->second.from_sai_type)(&sai_attr)) {
            NDI_LOG_TRACE("NDI-SAI", "Values are invalid - can't be converted from SAI types (func:%d)",attr);
            return STD_ERR(NPU,PARAM,0);
        }
    }

    switch(it->second.type) {
        case SW_ATTR_BOOL:
            param->s32 = sai_attr.value.booldata;
            break;
        case SW_ATTR_S32:
            param->s32 = sai_attr.value.s32;
            break;
        case SW_ATTR_U32:
            param->u32 = sai_attr.value.u32;
            break;
        case SW_ATTR_LST:
            param->list.len = sai_attr.value.s32list.count ;
            break;
        case SW_ATTR_U16:
            param->u16 = sai_attr.value.u16;
            break;
        case SW_ATTR_MAC:
            memcpy(param->mac, sai_attr.value.mac,sizeof(param->mac));
            break;
    }

    return STD_ERR_OK;
}

extern "C" t_std_error ndi_switch_mac_age_time_set(npu_id_t npu_id, uint32_t timeout_value)
{
    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr;

    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(npu_id);
    if (ndi_db_ptr == NULL) {
        NDI_LOG_TRACE("NDI-MAC", "Invalid npu id %d to set mac age timeout value",
                      npu_id);
        return STD_ERR(NPU, PARAM, 0);
    }

    memset(&sai_attr, 0, sizeof(sai_attribute_t));

    sai_attr.id = SAI_SWITCH_ATTR_FDB_AGING_TIME;
    sai_attr.value.u32 = (sai_uint32_t)timeout_value;

    if ((sai_ret = ndi_sai_switch_api_tbl_get(ndi_db_ptr)->set_switch_attribute(ndi_switch_id_get(),&sai_attr))
            != SAI_STATUS_SUCCESS) {
         return STD_ERR(MAC, CFG, sai_ret);
    }

    return STD_ERR_OK;
}

extern "C" t_std_error ndi_switch_mac_age_time_get(npu_id_t npu_id, uint32_t *timeout_value)
{
    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr;
    uint32_t attr_count = 1;

    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(npu_id);
    if (ndi_db_ptr == NULL) {
        NDI_LOG_TRACE("NDI-MAC", "Invalid npu id %d to get mac age timeout value",
                      npu_id);
        return STD_ERR(NPU, PARAM, 0);
    }

    memset(&sai_attr, 0, sizeof(sai_attribute_t));

    sai_attr.id = SAI_SWITCH_ATTR_FDB_AGING_TIME;

    if ((sai_ret = ndi_sai_switch_api_tbl_get(ndi_db_ptr)->get_switch_attribute(ndi_switch_id_get(),
                attr_count, &sai_attr)) != SAI_STATUS_SUCCESS) {
        NDI_LOG_TRACE("NDI-MAC", "Error from  SAI %d to get mac age timeout value",
                      sai_ret);
         return STD_ERR(MAC, CFG, sai_ret);
    }

    *timeout_value = (sai_uint32_t) sai_attr.value.u32;

    return STD_ERR_OK;
}

extern "C" t_std_error ndi_switch_set_sai_log_level(BASE_SWITCH_SUBSYSTEM_t api_id,
                                                    BASE_SWITCH_LOG_LEVEL_t level)
{
    sai_status_t sai_ret = SAI_STATUS_FAILURE;

    if(api_id == BASE_SWITCH_SUBSYSTEM_ALL){
        for(size_t ix = 0 ; ix < BASE_SWITCH_SUBSYSTEM_ALL ; ++ix){
            if ((sai_ret = sai_log_set((sai_api_t)ix,(sai_log_level_t)level))!= SAI_STATUS_SUCCESS){
                NDI_LOG_TRACE("NDI-DIAG", "Error from  SAI %d to set log level %d"
                              "for sai_module %d",sai_ret,level,api_id);
                return STD_ERR(DIAG, PARAM, sai_ret);
            }
        }
    }
    else{
        if ((sai_ret = sai_log_set((sai_api_t)api_id,(sai_log_level_t)level))!= SAI_STATUS_SUCCESS){
            NDI_LOG_TRACE("NDI-DIAG", "Error from  SAI %d to set log level %d"
                          "for sai_module %d",sai_ret,level,api_id);
            return STD_ERR(DIAG, PARAM, sai_ret);
        }
    }
    return STD_ERR_OK;
}

extern "C" t_std_error ndi_switch_get_queue_numbers(npu_id_t npu_id,
                        uint32_t *ucast_queues, uint32_t *mcast_queues,
                        uint32_t *total_queues, uint32_t *cpu_queues)
{
    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr[4];
    uint32_t attr_count = 4;

    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(npu_id);
    if (ndi_db_ptr == NULL) {
        NDI_LOG_TRACE("NDI-SWITCH-QOS", "Invalid npu id %d to get queue value",
                      npu_id);
        return STD_ERR(NPU, PARAM, 0);
    }

    memset(&sai_attr, 0, sizeof(sai_attr));

    sai_attr[0].id = SAI_SWITCH_ATTR_NUMBER_OF_UNICAST_QUEUES;
    sai_attr[1].id = SAI_SWITCH_ATTR_NUMBER_OF_MULTICAST_QUEUES;
    sai_attr[2].id = SAI_SWITCH_ATTR_NUMBER_OF_QUEUES;
    sai_attr[3].id = SAI_SWITCH_ATTR_NUMBER_OF_CPU_QUEUES;

    if ((sai_ret = ndi_sai_switch_api_tbl_get(ndi_db_ptr)->get_switch_attribute(ndi_switch_id_get(),
                attr_count, sai_attr)) != SAI_STATUS_SUCCESS) {
        NDI_LOG_TRACE("NDI-SWITCH-QOS", "Error from  SAI %d to get queue value",
                      sai_ret);
         return STD_ERR(NPU, CFG, sai_ret);
    }

    if (ucast_queues)
        *ucast_queues = (sai_uint32_t) sai_attr[0].value.u32;

    if (mcast_queues)
        *mcast_queues = (sai_uint32_t) sai_attr[1].value.u32;

    if (total_queues)
        *total_queues = (sai_uint32_t) sai_attr[2].value.u32;

    if (cpu_queues)
        *cpu_queues = (sai_uint32_t) sai_attr[3].value.u32;

    return STD_ERR_OK;
}

extern "C" t_std_error ndi_switch_get_max_number_of_scheduler_group_level(npu_id_t npu_id,
        uint32_t *max_level)
{
    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr[1];
    uint32_t attr_count = 1;

    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(npu_id);
    if (ndi_db_ptr == NULL) {
        NDI_LOG_TRACE("NDI-SWITCH-QOS", "Invalid npu id %d to get max scheduler group level",
                      npu_id);
        return STD_ERR(NPU, PARAM, 0);
    }

    memset(&sai_attr, 0, sizeof(sai_attr));

    sai_attr[0].id = SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUP_HIERARCHY_LEVELS;

    if ((sai_ret = ndi_sai_switch_api_tbl_get(ndi_db_ptr)->get_switch_attribute(ndi_switch_id_get(),
                attr_count, sai_attr)) != SAI_STATUS_SUCCESS) {
        NDI_LOG_TRACE("NDI-SWITCH-QOS", "Error from  SAI %d to get max scheduler group level",
                      sai_ret);
         return STD_ERR(NPU, CFG, sai_ret);
    }

    if (max_level)
        *max_level = (sai_uint32_t) sai_attr[0].value.u32;

    return STD_ERR_OK;

}

extern "C" t_std_error ndi_switch_get_slice_list(npu_id_t npu_id, nas_ndi_switch_param_t *param)
{
    size_t          copy_count;
    size_t          list_sz = param->obj_list.len;
    sai_attribute_t sai_attr = {.id = SAI_SWITCH_ATTR_EXTENSIONS_ACL_SLICE_LIST};
    sai_status_t    sai_ret;

    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(npu_id);
    if (ndi_db_ptr == NULL) {
        return STD_ERR(NPU, FAIL, 0);
    }
    std::vector<sai_object_id_t> shadow_slice_obj_list(list_sz);

    /* start with a size and resize it as required */
    sai_attr.value.objlist.count = param->obj_list.len;
    sai_attr.value.objlist.list = &(shadow_slice_obj_list[0]);

    NDI_LOG_INFO ("NDI-SWITCH", "ACL Slice list get, incoming list_len:%d, list_ptr:%lu",
            param->obj_list.len, param->obj_list.vals);

    sai_ret = ndi_sai_switch_api_tbl_get(ndi_db_ptr)->get_switch_attribute(ndi_switch_id_get(),
                                    1,&sai_attr);

    if (sai_ret == SAI_STATUS_BUFFER_OVERFLOW) {
        NDI_LOG_INFO ("NDI-SWITCH", "ACL Slice list get failed, input list len:%d "
                      "returned list len:%d SAI ret:%d",
                      param->obj_list.len, sai_attr.value.objlist.count, sai_ret);

        if (param->obj_list.vals == NULL) {
            param->obj_list.len = sai_attr.value.objlist.count;
            return STD_ERR_OK;
        }
    } else if (sai_ret != SAI_STATUS_SUCCESS) {
        NDI_LOG_ERROR("NDI-SWITCH", "ACL Slice list get failed in SAI:%d in get attrs (atleast id:%d) for NPU:%d",
                      sai_ret, sai_attr.id, (int)npu_id);

         return STD_ERR(NPU, CFG, sai_ret);
    }
    copy_count = sai_attr.value.objlist.count;

    /* if input list length is smaller than the actual list,
     * then copy only for the input length.
     */
    if (copy_count > param->obj_list.len) {
        copy_count = param->obj_list.len;
    }
    param->obj_list.len = sai_attr.value.objlist.count;

    for (size_t idx = 0; idx < copy_count; idx ++) {
        param->obj_list.vals[idx] = ndi_acl_sai2ndi_slice_id(sai_attr.value.objlist.list[idx]);
    }

    return STD_ERR_OK;
}

extern "C" bool nas2sai_switch_stats_type_get(BASE_SWITCH_SWITCHING_ENTITIES_SWITCH_STATS_t stat_id,
                                          sai_switch_attr_t *sai_stat_id)
{
    static const auto & nas2sai_switch_stats_type =
        * new std::unordered_map<BASE_SWITCH_SWITCHING_ENTITIES_SWITCH_STATS_t, sai_switch_attr_t,
                                std::hash<int>>
    {
        {BASE_SWITCH_SWITCHING_ENTITIES_SWITCH_STATS_AVAILABLE_IPV4_ROUTE_ENTRIES, SAI_SWITCH_ATTR_AVAILABLE_IPV4_ROUTE_ENTRY},
        {BASE_SWITCH_SWITCHING_ENTITIES_SWITCH_STATS_AVAILABLE_IPV6_ROUTE_ENTRIES, SAI_SWITCH_ATTR_AVAILABLE_IPV6_ROUTE_ENTRY},
        {BASE_SWITCH_SWITCHING_ENTITIES_SWITCH_STATS_AVAILABLE_IPV4_NEIGHBOUR_ENTRIES, SAI_SWITCH_ATTR_AVAILABLE_IPV4_NEIGHBOR_ENTRY},
        {BASE_SWITCH_SWITCHING_ENTITIES_SWITCH_STATS_AVAILABLE_IPV6_NEIGHBOUR_ENTRIES, SAI_SWITCH_ATTR_AVAILABLE_IPV6_NEIGHBOR_ENTRY},
        {BASE_SWITCH_SWITCHING_ENTITIES_SWITCH_STATS_AVAILABLE_FDB_ENTRIES, SAI_SWITCH_ATTR_AVAILABLE_FDB_ENTRY}
   };

    try {
        *sai_stat_id = nas2sai_switch_stats_type.at(stat_id);
    }
    catch (...) {
        EV_LOGGING(NDI, NOTICE, "NDI-SWITCH", "stats not mapped: stat_id %u",
                   stat_id);
        return false;
    }
    return true;
}

/**
 * This function gets the queue statistics
 * @param ndi_id
 * @param list of switch stats types to query
 * @param number of switch stats types specified
 * @param[out] counters: stats will be stored in the same order of the counter_ids
 * return standard error
 */
extern "C" t_std_error ndi_switch_get_statistics(npu_id_t npu_id,
                                BASE_SWITCH_SWITCHING_ENTITIES_SWITCH_STATS_t *counter_ids,
                                uint_t number_of_counters,
                                uint64_t *counters)
{
    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(npu_id);
    if (ndi_db_ptr == NULL) {
        NDI_LOG_TRACE("NDI-SWITCH",
                      "npu_id %d not exist\n", npu_id);
        return STD_ERR(NPU, PARAM, 0);
    }

    sai_attribute_t sai_attr_list[number_of_counters];
    sai_switch_attr_t sai_attr_id;
    uint_t i, j;
    int count = 0;

    for (i= 0; i < number_of_counters; i++) {
        if (nas2sai_switch_stats_type_get(counter_ids[i], &sai_attr_id)) {
            sai_attr_list[i].id = sai_attr_id;
            count++;
        } else {
            NDI_LOG_ERROR("NDI-SWITCH",
                    "NAS SWITCH Stat id %d is not mapped to any SAI stat id",
                    counter_ids[i]);
        }
    }

    if ((sai_ret = ndi_sai_switch_api_tbl_get(ndi_db_ptr)->get_switch_attribute(ndi_switch_id_get(),
                                             (uint32_t)count, sai_attr_list)) != SAI_STATUS_SUCCESS) {
        NDI_LOG_ERROR("NDI-SWITCH", "Error from SAI:%d in get attrs for NPU:%d",
                      sai_ret, (int)npu_id);
        return STD_ERR(NPU, CFG, sai_ret);
    }

    for (i= 0, j= 0; i < number_of_counters; i++) {
        if (nas2sai_switch_stats_type_get(counter_ids[i], &sai_attr_id)) {
            counters[i] = sai_attr_list[j].value.u32;
            j++;
        }
        else {
            // zero-filled for counters not able to poll
            counters[i] = 0;
        }
    }

    return STD_ERR_OK;
}


