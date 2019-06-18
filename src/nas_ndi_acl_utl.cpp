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
 * filename: nas_ndi_acl_utl.cpp
 */


#include "std_assert.h"
#include "std_mutex_lock.h"
#include "nas_ndi_int.h"
#include "nas_base_utils.h"
#include "nas_ndi_utils.h"
#include "nas_ndi_event_logs.h"
#include "nas_ndi_acl.h"
#include "nas_ndi_acl_utl.h"
#include <vector>
#include <unordered_map>
#include <string.h>
#include <list>
#include <netinet/in.h>

static std_mutex_lock_create_static_init_rec(table_lock);

const sai_acl_api_t* ndi_acl_utl_api_get (const nas_ndi_db_t* ndi_db_ptr)
{
    return(ndi_db_ptr->ndi_sai_api_tbl.n_sai_acl_api_tbl);
}

t_std_error ndi_acl_utl_ndi2sai_filter_type (BASE_ACL_MATCH_TYPE_t ndi_filter_type,
                                             sai_attribute_t* sai_attr_p)
{
    // Locking instances where global variables are used
    std_mutex_simple_lock_guard g(&table_lock);

    static const
        std::unordered_map<BASE_ACL_MATCH_TYPE_t, sai_acl_entry_attr_t, std::hash<int>>
        _nas2sai_entry_filter_type_map = {

            {BASE_ACL_MATCH_TYPE_SRC_IPV6,           SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6},
            {BASE_ACL_MATCH_TYPE_DST_IPV6,           SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6},
            {BASE_ACL_MATCH_TYPE_SRC_MAC,            SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC},
            {BASE_ACL_MATCH_TYPE_DST_MAC,            SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC},
            {BASE_ACL_MATCH_TYPE_SRC_IP,             SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP},
            {BASE_ACL_MATCH_TYPE_DST_IP,             SAI_ACL_ENTRY_ATTR_FIELD_DST_IP},
            {BASE_ACL_MATCH_TYPE_IN_PORTS,           SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS},
            {BASE_ACL_MATCH_TYPE_OUT_PORTS,          SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS},
            {BASE_ACL_MATCH_TYPE_IN_PORT,            SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT},
            {BASE_ACL_MATCH_TYPE_OUT_PORT,           SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT},
            {BASE_ACL_MATCH_TYPE_OUTER_VLAN_ID,      SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID},
            {BASE_ACL_MATCH_TYPE_OUTER_VLAN_PRI,     SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI},
            {BASE_ACL_MATCH_TYPE_OUTER_VLAN_CFI,     SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_CFI},
            {BASE_ACL_MATCH_TYPE_INNER_VLAN_ID,      SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID},
            {BASE_ACL_MATCH_TYPE_INNER_VLAN_PRI,     SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI},
            {BASE_ACL_MATCH_TYPE_INNER_VLAN_CFI,     SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_CFI},
            {BASE_ACL_MATCH_TYPE_L4_SRC_PORT,        SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT},
            {BASE_ACL_MATCH_TYPE_L4_DST_PORT,        SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT},
            {BASE_ACL_MATCH_TYPE_ETHER_TYPE,         SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE},
            {BASE_ACL_MATCH_TYPE_IP_PROTOCOL,        SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL},
            {BASE_ACL_MATCH_TYPE_DSCP,               SAI_ACL_ENTRY_ATTR_FIELD_DSCP},
            {BASE_ACL_MATCH_TYPE_ECN,                SAI_ACL_ENTRY_ATTR_FIELD_ECN},
            {BASE_ACL_MATCH_TYPE_TTL,                SAI_ACL_ENTRY_ATTR_FIELD_TTL},
            {BASE_ACL_MATCH_TYPE_TOS,                SAI_ACL_ENTRY_ATTR_FIELD_TOS},
            {BASE_ACL_MATCH_TYPE_IP_FLAGS,           SAI_ACL_ENTRY_ATTR_FIELD_IP_FLAGS},
            {BASE_ACL_MATCH_TYPE_TCP_FLAGS,          SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS},
            {BASE_ACL_MATCH_TYPE_IP_TYPE,            SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE},
            {BASE_ACL_MATCH_TYPE_IP_FRAG,            SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG},
            {BASE_ACL_MATCH_TYPE_IPV6_FLOW_LABEL,    SAI_ACL_ENTRY_ATTR_FIELD_IPV6_FLOW_LABEL},
            {BASE_ACL_MATCH_TYPE_TC,                 SAI_ACL_ENTRY_ATTR_FIELD_TC},
            {BASE_ACL_MATCH_TYPE_ICMP_TYPE,          SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE},
            {BASE_ACL_MATCH_TYPE_ICMP_CODE,          SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE},
            {BASE_ACL_MATCH_TYPE_SRC_PORT,           SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT},
            {BASE_ACL_MATCH_TYPE_NEIGHBOR_DST_HIT,   SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT},
            {BASE_ACL_MATCH_TYPE_ROUTE_DST_HIT,      SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_NPU_META_DST_HIT},
            {BASE_ACL_MATCH_TYPE_FDB_DST_HIT,        SAI_ACL_ENTRY_ATTR_FIELD_FDB_NPU_META_DST_HIT},
            {BASE_ACL_MATCH_TYPE_IN_INTFS,           SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS},
            {BASE_ACL_MATCH_TYPE_OUT_INTFS,          SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS},
            {BASE_ACL_MATCH_TYPE_IN_INTF,            SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT},
            {BASE_ACL_MATCH_TYPE_OUT_INTF,           SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT},
            {BASE_ACL_MATCH_TYPE_SRC_INTF,           SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT},
            {BASE_ACL_MATCH_TYPE_UDF,                SAI_ACL_ENTRY_ATTR_USER_DEFINED_FIELD_GROUP_MIN},
            {BASE_ACL_MATCH_TYPE_IPV6_NEXT_HEADER,   SAI_ACL_ENTRY_ATTR_FIELD_IPV6_NEXT_HEADER},
            {BASE_ACL_MATCH_TYPE_RANGE_CHECK,        SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE},
            {BASE_ACL_MATCH_TYPE_DROP_MARKED,        SAI_ACL_ENTRY_ATTR_FIELD_DROP_MARKED},
            {BASE_ACL_MATCH_TYPE_BRIDGE_TYPE,        SAI_ACL_ENTRY_ATTR_FIELD_BRIDGE_TYPE},
        };
    //Added for extension attrs
    static const
        std::unordered_map<BASE_ACL_MATCH_TYPE_t, sai_acl_entry_attr_extensions_t, std::hash<int>>
        _nas2sai_entry_extn_filter_type_map = {

            {BASE_ACL_MATCH_TYPE_MCAST_ROUTE_DST_HIT,SAI_ACL_ENTRY_ATTR_EXTENSIONS_FIELD_MCAST_ROUTE_NPU_META_DST_HIT},
            {BASE_ACL_MATCH_TYPE_ROUTER_INTERFACE_USER_MARK, SAI_ACL_ENTRY_ATTR_EXTENSIONS_FIELD_LAYER3_INTERFACE_USER_META},
        };

    auto type_map_itr = _nas2sai_entry_filter_type_map.find (ndi_filter_type);
    t_std_error rc = STD_ERR_OK;

    if (type_map_itr == _nas2sai_entry_filter_type_map.end()) {
        rc =  STD_ERR(ACL, PARAM, 0);
    }

    if (rc != STD_ERR_OK) {
       auto ext_type_map_itr = _nas2sai_entry_extn_filter_type_map.find (ndi_filter_type);

       if (ext_type_map_itr == _nas2sai_entry_extn_filter_type_map.end()) {
           return STD_ERR(ACL, PARAM, 0);
       }
       sai_attr_p->id = ext_type_map_itr->second;
    } else {
       sai_attr_p->id = type_map_itr->second;
    }

    return STD_ERR_OK;

}

t_std_error ndi_acl_utl_ndi2sai_action_type (BASE_ACL_ACTION_TYPE_t ndi_action_type,
                                             sai_attribute_t* sai_attr_p)
{
    // Locking instances where global variables are used
    std_mutex_simple_lock_guard g(&table_lock);

    static const
        std::unordered_map<BASE_ACL_ACTION_TYPE_t, sai_acl_entry_attr_t, std::hash<int>>
        _nas2sai_entry_action_type_map = {

            {BASE_ACL_ACTION_TYPE_PACKET_ACTION,        SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION},
            {BASE_ACL_ACTION_TYPE_FLOOD,                SAI_ACL_ENTRY_ATTR_ACTION_FLOOD},
            {BASE_ACL_ACTION_TYPE_MIRROR_INGRESS,       SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS},
            {BASE_ACL_ACTION_TYPE_MIRROR_EGRESS,        SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS},
            {BASE_ACL_ACTION_TYPE_SET_COUNTER,          SAI_ACL_ENTRY_ATTR_ACTION_COUNTER},
            {BASE_ACL_ACTION_TYPE_SET_POLICER,          SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER},
            {BASE_ACL_ACTION_TYPE_DECREMENT_TTL,        SAI_ACL_ENTRY_ATTR_ACTION_DECREMENT_TTL},
            {BASE_ACL_ACTION_TYPE_SET_TC,               SAI_ACL_ENTRY_ATTR_ACTION_SET_TC},
            {BASE_ACL_ACTION_TYPE_SET_INNER_VLAN_ID,    SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_ID},
            {BASE_ACL_ACTION_TYPE_SET_INNER_VLAN_PRI,   SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_PRI},
            {BASE_ACL_ACTION_TYPE_SET_OUTER_VLAN_ID,    SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID},
            {BASE_ACL_ACTION_TYPE_SET_OUTER_VLAN_PRI,   SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI},
            {BASE_ACL_ACTION_TYPE_SET_SRC_MAC,          SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_MAC},
            {BASE_ACL_ACTION_TYPE_SET_DST_MAC,          SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_MAC},
            {BASE_ACL_ACTION_TYPE_SET_SRC_IP,           SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IP},
            {BASE_ACL_ACTION_TYPE_SET_DST_IP,           SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IP},
            {BASE_ACL_ACTION_TYPE_SET_SRC_IPV6,         SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IPV6},
            {BASE_ACL_ACTION_TYPE_SET_DST_IPV6,         SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IPV6},
            {BASE_ACL_ACTION_TYPE_SET_DSCP,             SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP},
            {BASE_ACL_ACTION_TYPE_SET_L4_SRC_PORT,      SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_SRC_PORT},
            {BASE_ACL_ACTION_TYPE_SET_L4_DST_PORT,      SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_DST_PORT},
            {BASE_ACL_ACTION_TYPE_REDIRECT_PORT,        SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT},
            {BASE_ACL_ACTION_TYPE_REDIRECT_PORT_LIST,   SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT_LIST},
            {BASE_ACL_ACTION_TYPE_REDIRECT_IP_NEXTHOP,  SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT},
            {BASE_ACL_ACTION_TYPE_SET_CPU_QUEUE,        SAI_ACL_ENTRY_ATTR_ACTION_SET_CPU_QUEUE},
            {BASE_ACL_ACTION_TYPE_EGRESS_MASK,          SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_BLOCK_PORT_LIST},
            {BASE_ACL_ACTION_TYPE_REDIRECT_INTF,        SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT},
            {BASE_ACL_ACTION_TYPE_REDIRECT_INTF_LIST,   SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT_LIST},
            {BASE_ACL_ACTION_TYPE_EGRESS_INTF_MASK,     SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_BLOCK_PORT_LIST},
            {BASE_ACL_ACTION_TYPE_SET_USER_TRAP_ID,     SAI_ACL_ENTRY_ATTR_ACTION_SET_USER_TRAP_ID},
            {BASE_ACL_ACTION_TYPE_SET_PACKET_COLOR,     SAI_ACL_ENTRY_ATTR_ACTION_SET_PACKET_COLOR},
        };

    auto type_map_itr = _nas2sai_entry_action_type_map.find (ndi_action_type);

     if (type_map_itr == _nas2sai_entry_action_type_map.end()) {
        return STD_ERR(ACL, PARAM, 0);
    }

   sai_attr_p->id = type_map_itr->second;
   return STD_ERR_OK;
}

uint_t ndi_acl_utl_ndi2sai_action_type (BASE_ACL_ACTION_TYPE_t ndi_action_type)
{
    // Locking instances where global variables are used
    std_mutex_simple_lock_guard g(&table_lock);

    static const
        std::unordered_map<BASE_ACL_ACTION_TYPE_t, sai_acl_action_type_t, std::hash<int>>
        _nas2sai_action_type_map = {

            {BASE_ACL_ACTION_TYPE_REDIRECT_INTF,        SAI_ACL_ACTION_TYPE_REDIRECT},
            {BASE_ACL_ACTION_TYPE_REDIRECT_INTF_LIST,   SAI_ACL_ACTION_TYPE_REDIRECT_LIST},
            {BASE_ACL_ACTION_TYPE_REDIRECT_PORT,        SAI_ACL_ACTION_TYPE_REDIRECT},
            {BASE_ACL_ACTION_TYPE_REDIRECT_PORT_LIST,   SAI_ACL_ACTION_TYPE_REDIRECT_LIST},
            {BASE_ACL_ACTION_TYPE_REDIRECT_IP_NEXTHOP,  SAI_ACL_ACTION_TYPE_REDIRECT},
            {BASE_ACL_ACTION_TYPE_PACKET_ACTION,        SAI_ACL_ACTION_TYPE_PACKET_ACTION},
            {BASE_ACL_ACTION_TYPE_FLOOD,                SAI_ACL_ACTION_TYPE_FLOOD},
            {BASE_ACL_ACTION_TYPE_SET_COUNTER,          SAI_ACL_ACTION_TYPE_COUNTER},
            {BASE_ACL_ACTION_TYPE_MIRROR_INGRESS,       SAI_ACL_ACTION_TYPE_MIRROR_INGRESS},
            {BASE_ACL_ACTION_TYPE_MIRROR_EGRESS,        SAI_ACL_ACTION_TYPE_MIRROR_EGRESS},
            {BASE_ACL_ACTION_TYPE_SET_POLICER,          SAI_ACL_ACTION_TYPE_SET_POLICER},
            {BASE_ACL_ACTION_TYPE_DECREMENT_TTL,        SAI_ACL_ACTION_TYPE_DECREMENT_TTL},
            {BASE_ACL_ACTION_TYPE_SET_TC,               SAI_ACL_ACTION_TYPE_SET_TC},
            {BASE_ACL_ACTION_TYPE_SET_INNER_VLAN_ID,    SAI_ACL_ACTION_TYPE_SET_INNER_VLAN_ID},
            {BASE_ACL_ACTION_TYPE_SET_INNER_VLAN_PRI,   SAI_ACL_ACTION_TYPE_SET_INNER_VLAN_PRI},
            {BASE_ACL_ACTION_TYPE_SET_OUTER_VLAN_ID,    SAI_ACL_ACTION_TYPE_SET_OUTER_VLAN_ID},
            {BASE_ACL_ACTION_TYPE_SET_OUTER_VLAN_PRI,   SAI_ACL_ACTION_TYPE_SET_OUTER_VLAN_PRI},
            {BASE_ACL_ACTION_TYPE_SET_SRC_MAC,          SAI_ACL_ACTION_TYPE_SET_SRC_MAC},
            {BASE_ACL_ACTION_TYPE_SET_DST_MAC,          SAI_ACL_ACTION_TYPE_SET_DST_MAC},
            {BASE_ACL_ACTION_TYPE_SET_SRC_IP,           SAI_ACL_ACTION_TYPE_SET_SRC_IP},
            {BASE_ACL_ACTION_TYPE_SET_DST_IP,           SAI_ACL_ACTION_TYPE_SET_DST_IP},
            {BASE_ACL_ACTION_TYPE_SET_SRC_IPV6,         SAI_ACL_ACTION_TYPE_SET_SRC_IPV6},
            {BASE_ACL_ACTION_TYPE_SET_DST_IPV6,         SAI_ACL_ACTION_TYPE_SET_DST_IPV6},
            {BASE_ACL_ACTION_TYPE_SET_DSCP,             SAI_ACL_ACTION_TYPE_SET_DSCP},
            {BASE_ACL_ACTION_TYPE_SET_L4_SRC_PORT,      SAI_ACL_ACTION_TYPE_SET_L4_SRC_PORT},
            {BASE_ACL_ACTION_TYPE_SET_L4_DST_PORT,      SAI_ACL_ACTION_TYPE_SET_L4_DST_PORT},
            {BASE_ACL_ACTION_TYPE_SET_CPU_QUEUE,        SAI_ACL_ACTION_TYPE_SET_CPU_QUEUE},
            {BASE_ACL_ACTION_TYPE_EGRESS_MASK,          SAI_ACL_ACTION_TYPE_EGRESS_BLOCK_PORT_LIST},
            {BASE_ACL_ACTION_TYPE_EGRESS_INTF_MASK,     SAI_ACL_ACTION_TYPE_EGRESS_BLOCK_PORT_LIST},
            {BASE_ACL_ACTION_TYPE_SET_USER_TRAP_ID,     SAI_ACL_ACTION_TYPE_SET_USER_TRAP_ID},
            {BASE_ACL_ACTION_TYPE_SET_PACKET_COLOR,     SAI_ACL_ACTION_TYPE_SET_PACKET_COLOR},
        };

    auto type_map_itr = _nas2sai_action_type_map.find (ndi_action_type);

    if (type_map_itr == _nas2sai_action_type_map.end()) {
        throw std::out_of_range(std::string("Invalid action type id ") +
                                std::to_string(ndi_action_type));
    }

    return static_cast<uint_t>(type_map_itr->second);
}

// Map NAS-NDI Filter ID to SAI Table Filter ID
// Populate the SAI attribute
t_std_error ndi_acl_utl_ndi2sai_tbl_filter_type (BASE_ACL_MATCH_TYPE_t ndi_filter_type,
                                                 sai_attribute_t* sai_attr_p)
{
    // Locking instances where global variables are used
    std_mutex_simple_lock_guard g(&table_lock);

    static const
        std::unordered_map<BASE_ACL_MATCH_TYPE_t, sai_acl_table_attr_t, std::hash<int>>
        _nas2sai_tbl_filter_type_map = {

            {BASE_ACL_MATCH_TYPE_SRC_IPV6,           SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6},
            {BASE_ACL_MATCH_TYPE_DST_IPV6,           SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6},
            {BASE_ACL_MATCH_TYPE_SRC_MAC,            SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC},
            {BASE_ACL_MATCH_TYPE_DST_MAC,            SAI_ACL_TABLE_ATTR_FIELD_DST_MAC},
            {BASE_ACL_MATCH_TYPE_SRC_IP,             SAI_ACL_TABLE_ATTR_FIELD_SRC_IP},
            {BASE_ACL_MATCH_TYPE_DST_IP,             SAI_ACL_TABLE_ATTR_FIELD_DST_IP},
            {BASE_ACL_MATCH_TYPE_IN_PORTS,           SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS},
            {BASE_ACL_MATCH_TYPE_OUT_PORTS,          SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS},
            {BASE_ACL_MATCH_TYPE_IN_PORT,            SAI_ACL_TABLE_ATTR_FIELD_IN_PORT},
            {BASE_ACL_MATCH_TYPE_OUT_PORT,           SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT},
            {BASE_ACL_MATCH_TYPE_OUTER_VLAN_ID,      SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID},
            {BASE_ACL_MATCH_TYPE_OUTER_VLAN_PRI,     SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI},
            {BASE_ACL_MATCH_TYPE_OUTER_VLAN_CFI,     SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_CFI},
            {BASE_ACL_MATCH_TYPE_INNER_VLAN_ID,      SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID},
            {BASE_ACL_MATCH_TYPE_INNER_VLAN_PRI,     SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI},
            {BASE_ACL_MATCH_TYPE_INNER_VLAN_CFI,     SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI},
            {BASE_ACL_MATCH_TYPE_L4_SRC_PORT,        SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT},
            {BASE_ACL_MATCH_TYPE_L4_DST_PORT,        SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT},
            {BASE_ACL_MATCH_TYPE_ETHER_TYPE,         SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE},
            {BASE_ACL_MATCH_TYPE_IP_PROTOCOL,        SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL},
            {BASE_ACL_MATCH_TYPE_DSCP,               SAI_ACL_TABLE_ATTR_FIELD_DSCP},
            {BASE_ACL_MATCH_TYPE_ECN,                SAI_ACL_TABLE_ATTR_FIELD_ECN},
            {BASE_ACL_MATCH_TYPE_TTL,                SAI_ACL_TABLE_ATTR_FIELD_TTL},
            {BASE_ACL_MATCH_TYPE_TOS,                SAI_ACL_TABLE_ATTR_FIELD_TOS},
            {BASE_ACL_MATCH_TYPE_IP_FLAGS,           SAI_ACL_TABLE_ATTR_FIELD_IP_FLAGS},
            {BASE_ACL_MATCH_TYPE_TCP_FLAGS,          SAI_ACL_TABLE_ATTR_FIELD_TCP_FLAGS},
            {BASE_ACL_MATCH_TYPE_IP_TYPE,            SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE},
            {BASE_ACL_MATCH_TYPE_IP_FRAG,            SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_FRAG},
            {BASE_ACL_MATCH_TYPE_IPV6_FLOW_LABEL,    SAI_ACL_TABLE_ATTR_FIELD_IPV6_FLOW_LABEL},
            {BASE_ACL_MATCH_TYPE_TC,                 SAI_ACL_TABLE_ATTR_FIELD_TC},
            {BASE_ACL_MATCH_TYPE_ICMP_TYPE,          SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE},
            {BASE_ACL_MATCH_TYPE_ICMP_CODE,          SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE},
            {BASE_ACL_MATCH_TYPE_SRC_PORT,           SAI_ACL_TABLE_ATTR_FIELD_SRC_PORT},
            {BASE_ACL_MATCH_TYPE_NEIGHBOR_DST_HIT,   SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT},
            {BASE_ACL_MATCH_TYPE_ROUTE_DST_HIT,      SAI_ACL_TABLE_ATTR_FIELD_ROUTE_NPU_META_DST_HIT},
            {BASE_ACL_MATCH_TYPE_FDB_DST_HIT,        SAI_ACL_TABLE_ATTR_FIELD_FDB_NPU_META_DST_HIT},
            {BASE_ACL_MATCH_TYPE_IN_INTFS,           SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS},
            {BASE_ACL_MATCH_TYPE_OUT_INTFS,          SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS},
            {BASE_ACL_MATCH_TYPE_IN_INTF,            SAI_ACL_TABLE_ATTR_FIELD_IN_PORT},
            {BASE_ACL_MATCH_TYPE_OUT_INTF,           SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT},
            {BASE_ACL_MATCH_TYPE_SRC_INTF,           SAI_ACL_TABLE_ATTR_FIELD_SRC_PORT},
            {BASE_ACL_MATCH_TYPE_UDF,                SAI_ACL_TABLE_ATTR_USER_DEFINED_FIELD_GROUP_MIN},
            {BASE_ACL_MATCH_TYPE_IPV6_NEXT_HEADER,   SAI_ACL_TABLE_ATTR_FIELD_IPV6_NEXT_HEADER},
            {BASE_ACL_MATCH_TYPE_RANGE_CHECK,        SAI_ACL_TABLE_ATTR_FIELD_ACL_RANGE_TYPE},
            {BASE_ACL_MATCH_TYPE_DROP_MARKED,        SAI_ACL_TABLE_ATTR_FIELD_DROP_MARKED},
            {BASE_ACL_MATCH_TYPE_BRIDGE_TYPE,        SAI_ACL_TABLE_ATTR_FIELD_BRIDGE_TYPE},
       };

    static const
        std::unordered_map<BASE_ACL_MATCH_TYPE_t, sai_acl_table_attr_extensions_t, std::hash<int>>
        _nas2sai_tbl_extn_filter_type_map = {
            {BASE_ACL_MATCH_TYPE_MCAST_ROUTE_DST_HIT,SAI_ACL_TABLE_ATTR_EXTENSIONS_FIELD_MCAST_ROUTE_NPU_META_DST_HIT},
            {BASE_ACL_MATCH_TYPE_ROUTER_INTERFACE_USER_MARK, SAI_ACL_TABLE_ATTR_EXTENSIONS_FIELD_LAYER3_INTERFACE_USER_META},
       };

    auto filter_type_itr = _nas2sai_tbl_filter_type_map.find (ndi_filter_type);
    t_std_error rc = STD_ERR_OK;

    if (filter_type_itr == _nas2sai_tbl_filter_type_map.end()) {
        rc = STD_ERR(ACL, PARAM, 0);
    }

    if (rc != STD_ERR_OK) {
       auto ext_filter_type_itr = _nas2sai_tbl_extn_filter_type_map.find (ndi_filter_type);

       if (ext_filter_type_itr == _nas2sai_tbl_extn_filter_type_map.end()) {
           return STD_ERR(ACL, PARAM, 0);
       }
       sai_attr_p->id = ext_filter_type_itr->second;
    } else {
       sai_attr_p->id = filter_type_itr->second;
    }

    return STD_ERR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////
// Map NAS-NDI Filter values to SAI values and populate the SAI attribute
/////////////////////////////////////////////////////////////////////////////////////

static void _fill_sai_filter_ipv6_attr (sai_attribute_t *sai_attr_p,
                                        const ndi_acl_entry_filter_t* f,
                                        nas::mem_alloc_helper_t& mem_helper)
{
    auto& data = sai_attr_p->value.aclfield.data.ip6;
    auto& mask = sai_attr_p->value.aclfield.mask.ip6;
    memcpy((uint8_t *)&data, (uint8_t *)&f->data.values.ipv6, sizeof(data));
    memcpy((uint8_t *)&mask, (uint8_t *)&f->mask.values.ipv6, sizeof(mask));
}

static void _fill_sai_filter_ipv4_attr (sai_attribute_t *sai_attr_p,
                                        const ndi_acl_entry_filter_t* f,
                                        nas::mem_alloc_helper_t& mem_helper)
{
    auto& data = sai_attr_p->value.aclfield.data.ip4;
    auto& mask = sai_attr_p->value.aclfield.mask.ip4;
    memcpy((uint8_t *)&data, (uint8_t *)&f->data.values.ipv4, sizeof(data));
    memcpy((uint8_t *)&mask, (uint8_t *)&f->mask.values.ipv4, sizeof(mask));
}

static void _fill_sai_filter_mac_attr (sai_attribute_t *sai_attr_p,
                                        const ndi_acl_entry_filter_t* f,
                                        nas::mem_alloc_helper_t& mem_helper)
{
    auto& data = sai_attr_p->value.aclfield.data.mac;
    auto& mask = sai_attr_p->value.aclfield.mask.mac;

    memcpy((uint8_t *)&data, (uint8_t *)&f->data.values.mac, sizeof(data));
    memcpy((uint8_t *)&mask, (uint8_t *)&f->mask.values.mac, sizeof(mask));
}

static void _fill_sai_filter_portlist_attr (sai_attribute_t *sai_attr_p,
                                            const ndi_acl_entry_filter_t* f,
                                            nas::mem_alloc_helper_t& mem_helper)
{
    sai_object_id_t  sai_portid;
    size_t           portcount = f->data.values.ndi_portlist.port_count;

    sai_object_id_t* sai_portlist = mem_helper.alloc <sai_object_id_t> (portcount);

    for (uint_t count=0; count<portcount; count++) {
        auto npu_id = f->data.values.ndi_portlist.port_list[count].npu_id;
        auto npu_port = f->data.values.ndi_portlist.port_list[count].npu_port;

        if (ndi_sai_port_id_get (npu_id, npu_port, &sai_portid) != STD_ERR_OK) {
            throw std::out_of_range (std::string {"SAI port conversion failed for NPU "}
                                     + std::to_string (npu_id)
                                     + std::string {" Port "}
                                     + std::to_string (npu_port));
        }
        sai_portlist[count] = sai_portid;
        NDI_ACL_LOG_DETAIL ("Filter-Portlist: Fill SAI port %lu for NPU %d Port %d",
                            sai_portid, npu_id, npu_port);
    }

    sai_attr_p->value.aclfield.data.objlist.count = portcount;
    sai_attr_p->value.aclfield.data.objlist.list = sai_portlist;
}

static void _fill_sai_filter_port_attr (sai_attribute_t *sai_attr_p,
                                        const ndi_acl_entry_filter_t* f,
                                        nas::mem_alloc_helper_t& mem_helper)
{
    sai_object_id_t  sai_portid;

    auto npu_id = f->data.values.ndi_port.npu_id;
    auto npu_port = f->data.values.ndi_port.npu_port;

    if (ndi_sai_port_id_get (npu_id, npu_port, &sai_portid) != STD_ERR_OK) {
        throw std::out_of_range (std::string {"SAI port conversion failed for NPU "}
        + std::to_string (npu_id)
        + std::string {" Port "}
        + std::to_string (npu_port));
    }
    NDI_ACL_LOG_DETAIL ("Filter-Port: Fill SAI port %lu for NPU %d Port %d",
                        sai_portid, npu_id, npu_port);

    sai_attr_p->value.aclfield.data.oid = sai_portid;
}

static void _fill_sai_filter_u32 (sai_attribute_t *sai_attr_p,
                                  const ndi_acl_entry_filter_t* f,
                                  nas::mem_alloc_helper_t& mem_helper)
{
    sai_attr_p->value.aclfield.data.u32 = f->data.values.u32;
    sai_attr_p->value.aclfield.mask.u32 = f->mask.values.u32;
}

static void _fill_sai_filter_u16 (sai_attribute_t *sai_attr_p,
                                  const ndi_acl_entry_filter_t* f,
                                  nas::mem_alloc_helper_t& mem_helper)
{
    sai_attr_p->value.aclfield.data.u16 = f->data.values.u16;
    sai_attr_p->value.aclfield.mask.u16 = f->mask.values.u16;
}

static void _fill_sai_filter_u8 (sai_attribute_t *sai_attr_p,
                                 const ndi_acl_entry_filter_t* f,
                                 nas::mem_alloc_helper_t& mem_helper)
{
    sai_attr_p->value.aclfield.data.u8 = f->data.values.u8;
    sai_attr_p->value.aclfield.mask.u8 = f->mask.values.u8;
}

static void _fill_sai_filter_u8list (sai_attribute_t *sai_attr_p,
                                     const ndi_acl_entry_filter_t* f,
                                     nas::mem_alloc_helper_t& mem_helper)
{
    size_t bytecount = f->data.values.ndi_u8list.byte_count;
    uint8_t* datalist = mem_helper.alloc <uint8_t> (bytecount);
    uint8_t* masklist = mem_helper.alloc <uint8_t> (bytecount);

    for (uint_t count = 0; count < bytecount; count ++) {
        datalist[count] = f->data.values.ndi_u8list.byte_list[count];
        masklist[count] = f->mask.values.ndi_u8list.byte_list[count];
    }
    sai_attr_p->value.aclfield.data.u8list.count = bytecount;
    sai_attr_p->value.aclfield.data.u8list.list = datalist;
    sai_attr_p->value.aclfield.mask.u8list.count = bytecount;
    sai_attr_p->value.aclfield.mask.u8list.list = masklist;
}

static void _fill_sai_filter_ip_type (sai_attribute_t *sai_attr_p,
                                      const ndi_acl_entry_filter_t* f,
                                      nas::mem_alloc_helper_t& mem_helper)
{
    // Locking instances where global variables are used
    std_mutex_simple_lock_guard g(&table_lock);

    static const
        std::unordered_map <BASE_ACL_MATCH_IP_TYPE_t, sai_acl_ip_type_t,
                            std::hash<int>>
        nas2sai_iptype_map =
        {
            {BASE_ACL_MATCH_IP_TYPE_ANY,          SAI_ACL_IP_TYPE_ANY},
            {BASE_ACL_MATCH_IP_TYPE_IP,           SAI_ACL_IP_TYPE_IP},
            {BASE_ACL_MATCH_IP_TYPE_NON_IP,       SAI_ACL_IP_TYPE_NON_IP},
            {BASE_ACL_MATCH_IP_TYPE_IPV4ANY,      SAI_ACL_IP_TYPE_IPV4ANY},
            {BASE_ACL_MATCH_IP_TYPE_NON_IPV4,     SAI_ACL_IP_TYPE_NON_IPV4},
            {BASE_ACL_MATCH_IP_TYPE_IPV6ANY,      SAI_ACL_IP_TYPE_IPV6ANY},
            {BASE_ACL_MATCH_IP_TYPE_NON_IPV6,     SAI_ACL_IP_TYPE_NON_IPV6},
            {BASE_ACL_MATCH_IP_TYPE_ARP,          SAI_ACL_IP_TYPE_ARP},
            {BASE_ACL_MATCH_IP_TYPE_ARP_REQUEST,  SAI_ACL_IP_TYPE_ARP_REQUEST},
            {BASE_ACL_MATCH_IP_TYPE_ARP_REPLY,    SAI_ACL_IP_TYPE_ARP_REPLY}
        };

    auto it = nas2sai_iptype_map.find (f->data.ip_type);

    if (it == nas2sai_iptype_map.end()) {
        throw std::out_of_range (std::string {"Invalid IP type"}
                                 + std::to_string (f->data.ip_type));
    }
    sai_attr_p->value.aclfield.data.s32 = it->second;
}

static void _fill_sai_filter_ip_frag (sai_attribute_t *sai_attr_p,
                                      const ndi_acl_entry_filter_t* f,
                                      nas::mem_alloc_helper_t& mem_helper)
{
    // Locking instances where global variables are used
    std_mutex_simple_lock_guard g(&table_lock);

    static const
        std::unordered_map <BASE_ACL_MATCH_IP_FRAG_t, sai_acl_ip_frag_t,
                            std::hash<int>>
        nas2sai_ipfrag_map =
        {
            {BASE_ACL_MATCH_IP_FRAG_ANY,               SAI_ACL_IP_FRAG_ANY},
            {BASE_ACL_MATCH_IP_FRAG_NON_FRAG,          SAI_ACL_IP_FRAG_NON_FRAG},
            {BASE_ACL_MATCH_IP_FRAG_NON_FRAG_OR_HEAD,  SAI_ACL_IP_FRAG_NON_FRAG_OR_HEAD},
            {BASE_ACL_MATCH_IP_FRAG_HEAD,              SAI_ACL_IP_FRAG_HEAD},
            {BASE_ACL_MATCH_IP_FRAG_NON_HEAD,          SAI_ACL_IP_FRAG_NON_HEAD},
        };

    auto it = nas2sai_ipfrag_map.find (f->data.ip_frag);

    if (it == nas2sai_ipfrag_map.end()) {
        throw std::out_of_range (std::string {"Invalid IP Frag type"}
                                 + std::to_string (f->data.ip_frag));
    }
    sai_attr_p->value.aclfield.data.s32 = it->second;
}

static void _fill_sai_filter_oid (sai_attribute_t* sai_attr_p,
                                  const ndi_acl_entry_filter_t* ndi_filter_p,
                                  nas::mem_alloc_helper_t& mem_helper)
{
    auto sai_oid = static_cast<sai_object_id_t> (ndi_filter_p->data.values.ndi_obj_ref);
    sai_attr_p->value.aclfield.data.oid = sai_oid;
}

static void _fill_sai_filter_oid_list (sai_attribute_t* sai_attr_p,
                                       const ndi_acl_entry_filter_t* ndi_filter_p,
                                       nas::mem_alloc_helper_t& mem_helper)
{
    auto oid_count = ndi_filter_p->data.values.ndi_obj_ref_list.count;
    auto oid_list = mem_helper.alloc<sai_object_id_t> (oid_count);

    for (uint_t count=0; count < oid_count; count++) {
        oid_list [count] = static_cast<sai_object_id_t>
           (ndi_filter_p->data.values.ndi_obj_ref_list.list[count]);
    }

    sai_attr_p->value.aclfield.data.objlist.count = oid_count;
    sai_attr_p->value.aclfield.data.objlist.list = oid_list;
}

static void _fill_sai_filter_bool (sai_attribute_t* sai_attr_p,
                                   const ndi_acl_entry_filter_t* ndi_filter_p,
                                   nas::mem_alloc_helper_t& mem_helper)
{
    sai_attr_p->value.aclfield.data.booldata = ndi_filter_p->data.values.u32;
}

static void _fill_sai_filter_bridge_type (sai_attribute_t *sai_attr_p,
                                          const ndi_acl_entry_filter_t* f,
                                          nas::mem_alloc_helper_t& mem_helper)
{
    // Locking instances where global variables are used
    std_mutex_simple_lock_guard g(&table_lock);

    static const
        std::unordered_map <BASE_ACL_BRIDGE_TYPE_t, sai_bridge_type_t,
                            std::hash<int>>
        nas2sai_brtype_map =
        {
            {BASE_ACL_BRIDGE_TYPE_BRIDGE_1Q, SAI_BRIDGE_TYPE_1Q},
            {BASE_ACL_BRIDGE_TYPE_BRIDGE_1D, SAI_BRIDGE_TYPE_1D},
        };

    auto it = nas2sai_brtype_map.find(static_cast<BASE_ACL_BRIDGE_TYPE_t>(f->data.values.u32));

    if (it == nas2sai_brtype_map.end()) {
        throw std::out_of_range (std::string {"Invalid Bridge type"}
                                 + std::to_string (f->data.values.u32));
    }
    sai_attr_p->value.aclfield.data.s32 = it->second;
}

t_std_error ndi_acl_utl_fill_sai_filter (sai_attribute_t *sai_attr_p,
                                         const ndi_acl_entry_filter_t *ndi_filter_p,
                                         nas::mem_alloc_helper_t& mem_helper)
{
    // Locking instances where global variables are used
    std_mutex_simple_lock_guard g(&table_lock);

    typedef void (*fill_sai_filter_fn) (sai_attribute_t* s,
                                        const ndi_acl_entry_filter_t* f,
                                        nas::mem_alloc_helper_t& mem_helper);

    static const
        std::unordered_map<ndi_acl_filter_values_type_t, fill_sai_filter_fn, std::hash<int>>
        _fill_sai_filter_fn_map = {

            {NDI_ACL_FILTER_IP_TYPE,            _fill_sai_filter_ip_type},
            {NDI_ACL_FILTER_IP_FRAG,            _fill_sai_filter_ip_frag},
            {NDI_ACL_FILTER_PORTLIST,           _fill_sai_filter_portlist_attr},
            {NDI_ACL_FILTER_PORT,               _fill_sai_filter_port_attr},
            {NDI_ACL_FILTER_MAC_ADDR,           _fill_sai_filter_mac_attr},
            {NDI_ACL_FILTER_IPV4_ADDR,          _fill_sai_filter_ipv4_attr},
            {NDI_ACL_FILTER_IPV6_ADDR,          _fill_sai_filter_ipv6_attr},
            {NDI_ACL_FILTER_U32,                _fill_sai_filter_u32},
            {NDI_ACL_FILTER_U16,                _fill_sai_filter_u16},
            {NDI_ACL_FILTER_U8,                 _fill_sai_filter_u8},
            {NDI_ACL_FILTER_OBJ_ID,             _fill_sai_filter_oid},
            {NDI_ACL_FILTER_OBJ_ID_LIST,        _fill_sai_filter_oid_list},
            {NDI_ACL_FILTER_BOOL,               _fill_sai_filter_bool},
            {NDI_ACL_FILTER_U8LIST,             _fill_sai_filter_u8list},
            {NDI_ACL_FILTER_BRIDGE_TYPE,        _fill_sai_filter_bridge_type},
        };

    BASE_ACL_MATCH_TYPE_t filter_type        = ndi_filter_p->filter_type;

    // Filter ID
    auto rc = ndi_acl_utl_ndi2sai_filter_type (filter_type, sai_attr_p);
    if (rc  != STD_ERR_OK) {
        NDI_ACL_LOG_ERROR ("ACL filter type %d is not supported in SAI",
                           filter_type);
        return rc;
    }
    if (filter_type == BASE_ACL_MATCH_TYPE_UDF) {
        sai_attr_p->id += ndi_filter_p->udf_seq_no;
    }

    try {
        // Filter value
        auto fn_set_filter = _fill_sai_filter_fn_map.at (ndi_filter_p->values_type);
        fn_set_filter (sai_attr_p, ndi_filter_p, mem_helper);

    } catch (std::out_of_range& e) {
        NDI_ACL_LOG_ERROR ("Failed to fill SAI Attr for filter %d - %s",
                           filter_type, e.what());
        return STD_ERR(ACL, PARAM, 0);
    }

    return STD_ERR_OK;
}

///////////////////////////////////////////////////////////////////////////
// Map NAS-NDI Action values to SAI values and populate the SAI attribute
//////////////////////////////////////////////////////////////////////////

static void _fill_sai_action_oid (sai_attribute_t* sai_attr_p,
                                  const ndi_acl_entry_action_t* ndi_action_p,
                                  nas::mem_alloc_helper_t& mem_helper)
{
    auto sai_oid = static_cast<sai_object_id_t> (ndi_action_p->values.ndi_obj_ref);
    sai_attr_p->value.aclaction.parameter.oid = sai_oid;
}

static void _fill_sai_action_oid_list (sai_attribute_t* sai_attr_p,
                                       const ndi_acl_entry_action_t* ndi_action_p,
                                       nas::mem_alloc_helper_t& mem_helper)
{
    auto oid_count = ndi_action_p->values.ndi_obj_ref_list.count;
    auto oid_list = mem_helper.alloc<sai_object_id_t> (oid_count);

    for (uint_t count=0; count < oid_count; count++) {
        oid_list [count] = static_cast<sai_object_id_t>
           (ndi_action_p->values.ndi_obj_ref_list.list[count]);
    }

    sai_attr_p->value.aclaction.parameter.objlist.count = oid_count;
    sai_attr_p->value.aclaction.parameter.objlist.list = oid_list;
}

static void _fill_sai_action_set_u64 (sai_attribute_t* sai_attr_p,
                                     const ndi_acl_entry_action_t* ndi_action_p,
                                     nas::mem_alloc_helper_t& mem_helper)
{
    sai_attr_p->value.aclaction.parameter.oid = ndi_action_p->values.u64;
}

static void _fill_sai_action_set_u32 (sai_attribute_t* sai_attr_p,
                                      const ndi_acl_entry_action_t* ndi_action_p,
                                      nas::mem_alloc_helper_t& mem_helper)
{
    sai_attr_p->value.aclaction.parameter.u32 = ndi_action_p->values.u32;
}

static void _fill_sai_action_set_u16 (sai_attribute_t* sai_attr_p,
                                      const ndi_acl_entry_action_t* ndi_action_p,
                                      nas::mem_alloc_helper_t& mem_helper)
{
    sai_attr_p->value.aclaction.parameter.u16 = ndi_action_p->values.u16;
}

static void _fill_sai_action_set_u8 (sai_attribute_t* sai_attr_p,
                                     const ndi_acl_entry_action_t* ndi_action_p,
                                     nas::mem_alloc_helper_t& mem_helper)
{
    sai_attr_p->value.aclaction.parameter.u8 = ndi_action_p->values.u8;
}

static void _fill_sai_action_set_mac (sai_attribute_t* sai_attr_p,
                                          const ndi_acl_entry_action_t* ndi_action_p,
                                          nas::mem_alloc_helper_t& mem_helper)
{
    auto& data = sai_attr_p->value.aclaction.parameter.mac;
    memcpy((uint8_t *)&data, (uint8_t *)&ndi_action_p->values.mac, sizeof(data));
}

static void _fill_sai_action_set_ipv6 (sai_attribute_t* sai_attr_p,
                                       const ndi_acl_entry_action_t* ndi_action_p,
                                       nas::mem_alloc_helper_t& mem_helper)
{
    auto& data = sai_attr_p->value.aclaction.parameter.ip6;
    memcpy((uint8_t *)&data, (uint8_t *)&ndi_action_p->values.ipv6, sizeof(data));
}

static void _fill_sai_action_set_ipv4 (sai_attribute_t* sai_attr_p,
                                       const ndi_acl_entry_action_t* ndi_action_p,
                                       nas::mem_alloc_helper_t& mem_helper)
{
    auto& data = sai_attr_p->value.aclaction.parameter.ip4;
    memcpy((uint8_t *)&data, (uint8_t *)&ndi_action_p->values.ipv4, sizeof(data));
}

static void _fill_sai_action_set_npu_port (sai_attribute_t* sai_attr_p,
                                           const ndi_acl_entry_action_t* ndi_action_p,
                                           nas::mem_alloc_helper_t& mem_helper)
{
    sai_object_id_t  sai_portid;
    auto npu_id = ndi_action_p->values.ndi_port.npu_id;
    auto npu_port = ndi_action_p->values.ndi_port.npu_port;

    if (ndi_sai_port_id_get (npu_id, npu_port, &sai_portid) != STD_ERR_OK) {

        throw std::out_of_range {std::string {"SAI port conversion failed for NPU "} +
                                 std::to_string (npu_id) + std::string {" Port "} +
                                 std::to_string (npu_port)};
    }
    NDI_ACL_LOG_DETAIL ("Action-Port: Fill SAI port %lu for NPU %d Port %d",
                        sai_portid, npu_id, npu_port);

    sai_attr_p->value.aclaction.parameter.oid = sai_portid;
}

static void _fill_sai_action_set_npu_portlist (sai_attribute_t* sai_attr_p,
                                               const ndi_acl_entry_action_t* ndi_action_p,
                                               nas::mem_alloc_helper_t& mem_helper)
{
    sai_object_id_t  sai_portid;
    size_t           portcount = ndi_action_p->values.ndi_portlist.port_count;

    sai_object_id_t* sai_portlist = mem_helper.alloc <sai_object_id_t> (portcount);

    for (uint_t count=0; count<portcount; count++) {
        auto npu_id = ndi_action_p->values.ndi_portlist.port_list[count].npu_id;
        auto npu_port = ndi_action_p->values.ndi_portlist.port_list[count].npu_port;

        if (ndi_sai_port_id_get (npu_id, npu_port, &sai_portid) != STD_ERR_OK) {
            throw std::out_of_range (std::string {"SAI port conversion failed for NPU "}
                                     + std::to_string (npu_id)
                                     + std::string {" Port "}
                                     + std::to_string (npu_port));
        }
        sai_portlist[count] = sai_portid;
        NDI_ACL_LOG_DETAIL ("Action-Portlist: Fill SAI port %lu for NPU %d Port %d",
                            sai_portid, npu_id, npu_port);
    }

    sai_attr_p->value.aclaction.parameter.objlist.count = portcount;
    sai_attr_p->value.aclaction.parameter.objlist.list = sai_portlist;
}

static void _fill_sai_action_pkt_color (sai_attribute_t* sai_attr_p,
                                        const ndi_acl_entry_action_t* ndi_action_p,
                                        nas::mem_alloc_helper_t& mem_helper)
{
    static const
        std::unordered_map<BASE_ACL_PACKET_COLOR_t, sai_packet_color_t, std::hash<int>>
        ndi2sai_pkt_color_map = {
            {BASE_ACL_PACKET_COLOR_GREEN,       SAI_PACKET_COLOR_GREEN},
            {BASE_ACL_PACKET_COLOR_YELLOW,      SAI_PACKET_COLOR_YELLOW},
            {BASE_ACL_PACKET_COLOR_RED,         SAI_PACKET_COLOR_RED},
        };

    auto it = ndi2sai_pkt_color_map.find (ndi_action_p->pkt_color);

    if (it == ndi2sai_pkt_color_map.end()) {
        throw std::out_of_range (std::string {"Invalid packet color type"}
                                 + std::to_string (ndi_action_p->pkt_color));
    }
    sai_attr_p->value.aclaction.parameter.s32 = it->second;
}

static void _fill_sai_action_pkt_action (sai_attribute_t* sai_attr_p,
                                         const ndi_acl_entry_action_t* ndi_action_p,
                                         nas::mem_alloc_helper_t& mem_helper)
{
    // Locking instances where global variables are used
    std_mutex_simple_lock_guard g(&table_lock);

    static const
        std::unordered_map<BASE_ACL_PACKET_ACTION_TYPE_t, sai_packet_action_t, std::hash<int>>
        ndi2sai_pkt_action_map = {

            {BASE_ACL_PACKET_ACTION_TYPE_FORWARD,       SAI_PACKET_ACTION_FORWARD},
            {BASE_ACL_PACKET_ACTION_TYPE_DROP,          SAI_PACKET_ACTION_DROP},
            {BASE_ACL_PACKET_ACTION_TYPE_COPY_TO_CPU,   SAI_PACKET_ACTION_COPY},
            {BASE_ACL_PACKET_ACTION_TYPE_TRAP_TO_CPU,   SAI_PACKET_ACTION_TRAP},
            {BASE_ACL_PACKET_ACTION_TYPE_COPY_TO_CPU_CANCEL,   SAI_PACKET_ACTION_COPY_CANCEL},
            {BASE_ACL_PACKET_ACTION_TYPE_COPY_TO_CPU_AND_FORWARD,    SAI_PACKET_ACTION_LOG},
            {BASE_ACL_PACKET_ACTION_TYPE_COPY_TO_CPU_CANCEL_AND_DROP,   SAI_PACKET_ACTION_DENY},
            {BASE_ACL_PACKET_ACTION_TYPE_COPY_TO_CPU_CANCEL_AND_FORWARD,   SAI_PACKET_ACTION_TRANSIT},
        };

    auto it = ndi2sai_pkt_action_map.find (ndi_action_p->pkt_action);

    if (it == ndi2sai_pkt_action_map.end()) {
        throw std::out_of_range (std::string {"Invalid packet action type"}
                                 + std::to_string (ndi_action_p->pkt_action));
    }
    sai_attr_p->value.aclaction.parameter.s32 = it->second;
}

t_std_error ndi_acl_utl_fill_sai_action (sai_attribute_t* sai_attr_p,
                                         const ndi_acl_entry_action_t* ndi_action_p,
                                         nas::mem_alloc_helper_t& mem_helper)
{
    // Locking instances where global variables are used
    std_mutex_simple_lock_guard g(&table_lock);

    typedef void (*fill_sai_action_fn) (sai_attribute_t* s,
                                        const ndi_acl_entry_action_t* a,
                                        nas::mem_alloc_helper_t& m);

    static const
        std::unordered_map<ndi_acl_action_values_type_t, fill_sai_action_fn, std::hash<int>>
        _fill_sai_action_fn_map = {

            {NDI_ACL_ACTION_NO_VALUE,           NULL},
            {NDI_ACL_ACTION_PKT_ACTION,        _fill_sai_action_pkt_action},
            {NDI_ACL_ACTION_OBJ_ID,            _fill_sai_action_oid},
            {NDI_ACL_ACTION_OBJ_ID_LIST,       _fill_sai_action_oid_list},
            {NDI_ACL_ACTION_PORT,              _fill_sai_action_set_npu_port},
            {NDI_ACL_ACTION_PORTLIST,          _fill_sai_action_set_npu_portlist},
            {NDI_ACL_ACTION_MAC_ADDR,          _fill_sai_action_set_mac},
            {NDI_ACL_ACTION_IPV4_ADDR,         _fill_sai_action_set_ipv4},
            {NDI_ACL_ACTION_IPV6_ADDR,         _fill_sai_action_set_ipv6},
            {NDI_ACL_ACTION_U32,               _fill_sai_action_set_u32},
            {NDI_ACL_ACTION_U16,               _fill_sai_action_set_u16},
            {NDI_ACL_ACTION_U8,                _fill_sai_action_set_u8},
            {NDI_ACL_ACTION_PKT_COLOR,         _fill_sai_action_pkt_color},
            {NDI_ACL_ACTION_U64,               _fill_sai_action_set_u64},
        };

    BASE_ACL_ACTION_TYPE_t  action_type = ndi_action_p->action_type;
    // Action ID
    auto rc = ndi_acl_utl_ndi2sai_action_type (action_type, sai_attr_p);
    if (rc != STD_ERR_OK) {
        NDI_ACL_LOG_ERROR ("ACL action type %d is not supported in SAI",
                           action_type);
        return rc;
    }

    try {
        // Action value
        auto fn_set_action = _fill_sai_action_fn_map.at (ndi_action_p->values_type);
        if (fn_set_action) {
            fn_set_action (sai_attr_p, ndi_action_p, mem_helper);
        }

    } catch (std::out_of_range& e) {
        NDI_ACL_LOG_ERROR ("Failed to fill SAI Attr for action %d - %s",
                           action_type, e.what());
        return STD_ERR(ACL, PARAM, 0);
    }

    return STD_ERR_OK;
}

//////////////////////////////
// Set/Get Counter attributes
//////////////////////////////

t_std_error ndi_acl_utl_set_counter_attr (npu_id_t npu_id,
                                          ndi_obj_id_t ndi_counter_id,
                                          const sai_attribute_t* sai_counter_attr_p)
{
    sai_status_t      sai_ret = SAI_STATUS_FAILURE;
    nas_ndi_db_t     *ndi_db_ptr = ndi_db_ptr_get(npu_id);

    if (ndi_db_ptr == NULL) {
        return STD_ERR (ACL, FAIL, 0);
    }

    sai_object_id_t sai_counter_id = ndi_acl_utl_ndi2sai_counter_id (ndi_counter_id);

    // Call SAI API
    if ((sai_ret = ndi_acl_utl_api_get (ndi_db_ptr)->set_acl_counter_attribute (sai_counter_id,
                                                                                sai_counter_attr_p))
        != SAI_STATUS_SUCCESS) {
        return STD_ERR (ACL, FAIL, sai_ret);
    }

    return STD_ERR_OK;
}

t_std_error ndi_acl_utl_get_counter_attr (npu_id_t npu_id,
                                          ndi_obj_id_t ndi_counter_id,
                                          sai_attribute_t* sai_counter_attr_p,
                                          size_t attr_cnt)
{
    sai_status_t      sai_ret = SAI_STATUS_FAILURE;
    nas_ndi_db_t     *ndi_db_ptr = ndi_db_ptr_get(npu_id);

    if (ndi_db_ptr == NULL) {
        return STD_ERR (ACL, FAIL, 0);
    }

    sai_object_id_t sai_counter_id = ndi_acl_utl_ndi2sai_counter_id (ndi_counter_id);

    // Call SAI API
    if ((sai_ret = ndi_acl_utl_api_get (ndi_db_ptr)->get_acl_counter_attribute (sai_counter_id,
                                                                                attr_cnt,
                                                                                sai_counter_attr_p))
        != SAI_STATUS_SUCCESS) {
        return STD_ERR(ACL, FAIL, sai_ret);
    }

    return STD_ERR_OK;
}
