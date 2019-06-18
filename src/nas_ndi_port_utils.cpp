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
 * filename: nas_ndi_port_utils.cpp
 */

#include "nas_ndi_port_utils.h"
#include<unordered_map>
#include <algorithm>


/* TODO: add SAI_BRIDGE_PORT_FDB_LEARNING_MODE_FDB_NOTIFICATION */
sai_bridge_port_fdb_learning_mode_t ndi_port_get_sai_mac_learn_mode
                             (BASE_IF_PHY_MAC_LEARN_MODE_t ndi_fdb_learn_mode){

    static const auto ndi_to_sai_fdb_learn_mode = new std::unordered_map<BASE_IF_PHY_MAC_LEARN_MODE_t,
                                                            sai_bridge_port_fdb_learning_mode_t,std::hash<int>>
    {
        {BASE_IF_PHY_MAC_LEARN_MODE_DROP, SAI_BRIDGE_PORT_FDB_LEARNING_MODE_DROP},
        {BASE_IF_PHY_MAC_LEARN_MODE_DISABLE, SAI_BRIDGE_PORT_FDB_LEARNING_MODE_DISABLE},
        {BASE_IF_PHY_MAC_LEARN_MODE_HW, SAI_BRIDGE_PORT_FDB_LEARNING_MODE_HW},
        {BASE_IF_PHY_MAC_LEARN_MODE_CPU_TRAP, SAI_BRIDGE_PORT_FDB_LEARNING_MODE_CPU_TRAP},
        {BASE_IF_PHY_MAC_LEARN_MODE_CPU_LOG, SAI_BRIDGE_PORT_FDB_LEARNING_MODE_CPU_LOG},
        {BASE_IF_PHY_MAC_LEARN_MODE_HW_DISABLE_ONLY, SAI_BRIDGE_PORT_FDB_LEARNING_MODE_DISABLE},
    };

    sai_bridge_port_fdb_learning_mode_t mode = SAI_BRIDGE_PORT_FDB_LEARNING_MODE_HW;

    auto it = ndi_to_sai_fdb_learn_mode->find(ndi_fdb_learn_mode);

    if(it != ndi_to_sai_fdb_learn_mode->end()){
        mode = it->second;
    }

    return mode;
}


BASE_IF_PHY_MAC_LEARN_MODE_t ndi_port_get_mac_learn_mode
                             (sai_bridge_port_fdb_learning_mode_t sai_fdb_learn_mode){

    static const auto sai_to_ndi_fdb_learn_mode = new std::unordered_map<sai_bridge_port_fdb_learning_mode_t, BASE_IF_PHY_MAC_LEARN_MODE_t,std::hash<int>>
    {
        {SAI_BRIDGE_PORT_FDB_LEARNING_MODE_DROP, BASE_IF_PHY_MAC_LEARN_MODE_DROP},
        {SAI_BRIDGE_PORT_FDB_LEARNING_MODE_DISABLE, BASE_IF_PHY_MAC_LEARN_MODE_DISABLE},
        {SAI_BRIDGE_PORT_FDB_LEARNING_MODE_HW, BASE_IF_PHY_MAC_LEARN_MODE_HW},
        {SAI_BRIDGE_PORT_FDB_LEARNING_MODE_CPU_TRAP, BASE_IF_PHY_MAC_LEARN_MODE_CPU_TRAP},
        {SAI_BRIDGE_PORT_FDB_LEARNING_MODE_CPU_LOG, BASE_IF_PHY_MAC_LEARN_MODE_CPU_LOG},
    };
    BASE_IF_PHY_MAC_LEARN_MODE_t mode = BASE_IF_PHY_MAC_LEARN_MODE_HW;

    auto it = sai_to_ndi_fdb_learn_mode->find(sai_fdb_learn_mode);

    if(it != sai_to_ndi_fdb_learn_mode->end()){
        mode = it->second;
    }

    return mode;
}


sai_port_internal_loopback_mode_t ndi_port_get_sai_loopback_mode
                             (BASE_CMN_LOOPBACK_TYPE_t lpbk_mode){

    static const auto ndi2sai_int_loopback_mode = new std::unordered_map<BASE_CMN_LOOPBACK_TYPE_t, sai_port_internal_loopback_mode_t,std::hash<int>>
    {
        {BASE_CMN_LOOPBACK_TYPE_NONE, SAI_PORT_INTERNAL_LOOPBACK_MODE_NONE},
        {BASE_CMN_LOOPBACK_TYPE_PHY, SAI_PORT_INTERNAL_LOOPBACK_MODE_PHY},
        {BASE_CMN_LOOPBACK_TYPE_MAC, SAI_PORT_INTERNAL_LOOPBACK_MODE_MAC},
    };

     sai_port_internal_loopback_mode_t mode = SAI_PORT_INTERNAL_LOOPBACK_MODE_NONE;

    auto it = ndi2sai_int_loopback_mode->find(lpbk_mode);

    if(it != ndi2sai_int_loopback_mode->end()){
        mode = it->second;
    }
    return mode;
}

BASE_CMN_LOOPBACK_TYPE_t ndi_port_get_ndi_loopback_mode(sai_port_internal_loopback_mode_t lpbk_mode){

    static const auto sai2ndi_int_loopback_mode = new std::unordered_map<sai_port_internal_loopback_mode_t,BASE_CMN_LOOPBACK_TYPE_t,std::hash<int>>
    {
        {SAI_PORT_INTERNAL_LOOPBACK_MODE_NONE, BASE_CMN_LOOPBACK_TYPE_NONE},
        {SAI_PORT_INTERNAL_LOOPBACK_MODE_PHY, BASE_CMN_LOOPBACK_TYPE_PHY},
        {SAI_PORT_INTERNAL_LOOPBACK_MODE_MAC, BASE_CMN_LOOPBACK_TYPE_MAC},
    };

    BASE_CMN_LOOPBACK_TYPE_t mode = BASE_CMN_LOOPBACK_TYPE_NONE;

    auto it = sai2ndi_int_loopback_mode->find(lpbk_mode);

    if(it != sai2ndi_int_loopback_mode->end()){
        mode = it->second;
    }
    return mode;
}


static const auto ndi2sai_speed = new std::unordered_map<BASE_IF_SPEED_t, uint32_t,std::hash<int>>
    {
        {BASE_IF_SPEED_10MBPS,       10},
        {BASE_IF_SPEED_100MBPS,     100},
        {BASE_IF_SPEED_1GIGE,      1000},
        {BASE_IF_SPEED_10GIGE,    10000},
        {BASE_IF_SPEED_20GIGE,    20000},
        {BASE_IF_SPEED_25GIGE,    25000},
        {BASE_IF_SPEED_40GIGE,    40000},
        {BASE_IF_SPEED_50GIGE,    50000},
        {BASE_IF_SPEED_100GIGE,  100000},
        {BASE_IF_SPEED_4GFC,       4000},
        {BASE_IF_SPEED_8GFC,       8000},
        {BASE_IF_SPEED_16GFC,     16000},
        {BASE_IF_SPEED_32GFC,     32000},
    };

bool ndi_port_get_sai_speed(BASE_IF_SPEED_t speed, uint32_t *sai_speed){

    auto it = ndi2sai_speed->find(speed);
    if(it == ndi2sai_speed->end()) return false;
    *sai_speed = it->second;
    return true;
}

bool ndi_port_get_ndi_speed(uint32_t sai_speed, BASE_IF_SPEED_t *ndi_speed) {
    auto it = std::find_if(ndi2sai_speed->begin(), ndi2sai_speed->end(),
            [&sai_speed](decltype(*ndi2sai_speed->begin()) &speed_map){return speed_map.second == sai_speed;});
    if (it == ndi2sai_speed->end()) return false;
    *ndi_speed = it->first;
    return true;
}



sai_port_fec_mode_t ndi_port_get_sai_fec_mode(BASE_CMN_FEC_TYPE_t ndi_fec_mode)
{
    static const auto ndi2sai_int_fec_mode = new std::unordered_map<BASE_CMN_FEC_TYPE_t, sai_port_fec_mode_t, std::hash<int>>
    {
        {BASE_CMN_FEC_TYPE_OFF, SAI_PORT_FEC_MODE_NONE},
        {BASE_CMN_FEC_TYPE_CL74_FC, SAI_PORT_FEC_MODE_FC},
        {BASE_CMN_FEC_TYPE_CL91_RS, SAI_PORT_FEC_MODE_RS},
        {BASE_CMN_FEC_TYPE_CL108_RS, SAI_PORT_FEC_MODE_RS}
    };

    auto it = ndi2sai_int_fec_mode->find(ndi_fec_mode);
    if (it == ndi2sai_int_fec_mode->end()) {
        return SAI_PORT_FEC_MODE_NONE;
    }

    return ndi2sai_int_fec_mode->at(ndi_fec_mode);
}

BASE_CMN_FEC_TYPE_t ndi_port_get_fec_mode(sai_port_fec_mode_t sai_fec_mode,
                                          bool supp_100g)
{
    if (sai_fec_mode == SAI_PORT_FEC_MODE_NONE) {
        return BASE_CMN_FEC_TYPE_OFF;
    }
    if (sai_fec_mode == SAI_PORT_FEC_MODE_FC) {
        return BASE_CMN_FEC_TYPE_CL74_FC;
    }

    //RS mode depends on port speed
    if (supp_100g) {
        return BASE_CMN_FEC_TYPE_CL91_RS;
    } else {
        return BASE_CMN_FEC_TYPE_CL108_RS;
    }
}
