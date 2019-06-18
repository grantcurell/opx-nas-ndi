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
 * filename: nas_ndi_mac_utl.cpp
 */

#include "std_error_codes.h"
#include "ds_common_types.h"
#include "nas_ndi_event_logs.h"
#include "nas_ndi_mac.h"
#include "nas_ndi_utils.h"
#include "nas_ndi_mac_utl.h"
#include "sai.h"
#include "saistatus.h"
#include "saitypes.h"

#include<unordered_map>


/*  NDI L2 MAC Utility APIs  */
sai_packet_action_t ndi_mac_sai_packet_action_get(BASE_MAC_PACKET_ACTION_t action)
{
    static const auto ndi_to_sai_packet_action = new std::unordered_map<BASE_MAC_PACKET_ACTION_t, sai_packet_action_t, std::hash<int>>
    {
        {BASE_MAC_PACKET_ACTION_FORWARD, SAI_PACKET_ACTION_FORWARD},
        {BASE_MAC_PACKET_ACTION_TRAP, SAI_PACKET_ACTION_TRAP},
        {BASE_MAC_PACKET_ACTION_DROP, SAI_PACKET_ACTION_DROP},
        {BASE_MAC_PACKET_ACTION_LOG, SAI_PACKET_ACTION_LOG}
    };

    sai_packet_action_t sai_action = SAI_PACKET_ACTION_FORWARD;

    auto it = ndi_to_sai_packet_action->find(action);

    if (it != ndi_to_sai_packet_action->end()) {
        sai_action = it->second;
    } else {
        EV_LOG(INFO, NDI, ev_log_s_MINOR, "NDI-MAC", "Could not translate ndi to sai packet action");
    }

    return sai_action;
}


BASE_MAC_PACKET_ACTION_t ndi_mac_packet_action_get(sai_packet_action_t action)
{
    static const auto sai_to_ndi_packet_action = new std::unordered_map<sai_packet_action_t, BASE_MAC_PACKET_ACTION_t, std::hash<int>>
    {
        {SAI_PACKET_ACTION_FORWARD, BASE_MAC_PACKET_ACTION_FORWARD},
        {SAI_PACKET_ACTION_TRAP, BASE_MAC_PACKET_ACTION_TRAP},
        {SAI_PACKET_ACTION_DROP, BASE_MAC_PACKET_ACTION_DROP},
        {SAI_PACKET_ACTION_LOG, BASE_MAC_PACKET_ACTION_LOG}
    };

    BASE_MAC_PACKET_ACTION_t packet_action = BASE_MAC_PACKET_ACTION_FORWARD;

    auto it = sai_to_ndi_packet_action->find(action);

    if (it != sai_to_ndi_packet_action->end()) {
        packet_action = it->second;
    } else {
        EV_LOG(INFO, NDI, ev_log_s_MINOR, "NDI-MAC", "Could not translate sai to ndi packet action");
    }

    return packet_action;
}

ndi_mac_event_type_t ndi_mac_event_type_get(sai_fdb_event_t sai_event_type)
{
    static const auto sai_to_ndi_event_type = new std::unordered_map<sai_fdb_event_t, ndi_mac_event_type_t, std::hash<int>>
    {
        {SAI_FDB_EVENT_LEARNED, NDI_MAC_EVENT_LEARNED},
        {SAI_FDB_EVENT_AGED, NDI_MAC_EVENT_AGED},
        {SAI_FDB_EVENT_FLUSHED, NDI_MAC_EVENT_FLUSHED},
        {SAI_FDB_EVENT_MOVE,NDI_MAC_EVENT_MOVED}
    };

    ndi_mac_event_type_t event_type = NDI_MAC_EVENT_INVALID;

    auto it = sai_to_ndi_event_type->find(sai_event_type);

    if (it != sai_to_ndi_event_type->end()) {
        event_type = it->second;
    } else {
        EV_LOG(INFO, NDI, ev_log_s_MINOR, "NDI-MAC", "Could not translate ndi to mac event type");
    }

    return event_type;
}
