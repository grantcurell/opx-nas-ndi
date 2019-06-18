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
 * filename: nas_ndi_qos_priority_group.cpp
 */

#include "std_error_codes.h"
#include "nas_ndi_event_logs.h"
#include "nas_ndi_int.h"
#include "nas_ndi_utils.h"
#include "nas_ndi_qos_utl.h"
#include "sai.h"
#include "dell-base-qos.h" //from yang model
#include "nas_ndi_qos.h"

#include <stdio.h>
#include <vector>
#include <unordered_map>


/**
  * This function sets the priority_group profile attributes in the NPU.
  * @param ndi_port_id
  * @param ndi_priority_group_id
  * @param buffer_profile_id
  * @return standard error
  */
t_std_error ndi_qos_set_priority_group_buffer_profile_id(ndi_port_t ndi_port_id,
                                        ndi_obj_id_t ndi_priority_group_id,
                                        ndi_obj_id_t buffer_profile_id)
{
    sai_status_t sai_ret = SAI_STATUS_FAILURE;

    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(ndi_port_id.npu_id);
    if (ndi_db_ptr == NULL) {
        EV_LOGGING(NDI, DEBUG, "NDI-QOS",
                      "npu_id %d not exist\n", ndi_port_id.npu_id);
        return STD_ERR(QOS, CFG, 0);
    }

    sai_attribute_t sai_attr = {0};
    sai_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    sai_attr.value.oid = ndi2sai_buffer_profile_id(buffer_profile_id);

    sai_ret = ndi_sai_qos_buffer_api(ndi_db_ptr)->
            set_ingress_priority_group_attribute(
                    ndi2sai_priority_group_id(ndi_priority_group_id),
                    &sai_attr);

    if (sai_ret != SAI_STATUS_SUCCESS && sai_ret != SAI_STATUS_ITEM_ALREADY_EXISTS) {
        EV_LOGGING(NDI, NOTICE, "NDI-QOS",
                   "npu_id %d,  ndi_priority_group_id 0x%016lx, buffer profile 0x%016lx, "
                   "sai_buf_profile_id 0x%016lx set failed, rc= %d\n",
                   ndi_port_id.npu_id, ndi_priority_group_id,
                   buffer_profile_id, sai_attr.value.oid, sai_ret);
        return ndi_utl_mk_qos_std_err(sai_ret);
    }

    return STD_ERR_OK;
}

static t_std_error _fill_ndi_qos_priority_group_struct(sai_attribute_t *attr_list,
                        uint_t num_attr, ndi_qos_priority_group_attribute_t *p)
{

    for (uint_t i = 0 ; i< num_attr; i++ ) {
        sai_attribute_t *attr = &attr_list[i];
        if (attr->id == SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE)
            p->buffer_profile = sai2ndi_buffer_profile_id(attr->value.u64);
    }

    return STD_ERR_OK;
}

/**
 * This function get a priority_group from the NPU.
 * @param ndi_port_id
 * @param ndi_priority_group_id
 * @param[out] ndi_qos_priority_group_struct_t filled if success
 * @return standard error
 */
t_std_error ndi_qos_get_priority_group_attribute(ndi_port_t ndi_port_id,
                            ndi_obj_id_t ndi_priority_group_id,
                            ndi_qos_priority_group_attribute_t *p)
{
    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    std::vector<sai_attribute_t> attr_list;
    sai_attribute_t  sai_attr = {0};

    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(ndi_port_id.npu_id);
    if (ndi_db_ptr == NULL) {
        EV_LOGGING(NDI, DEBUG, "NDI-QOS",
                      "npu_id %d not exist\n", ndi_port_id.npu_id);
        return STD_ERR(QOS, CFG, 0);
    }

    sai_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    attr_list.push_back(sai_attr);

    if ((sai_ret = ndi_sai_qos_buffer_api(ndi_db_ptr)->
            get_ingress_priority_group_attribute(
                    ndi2sai_priority_group_id(ndi_priority_group_id),
                    attr_list.size(),
                    &attr_list[0]))
                         != SAI_STATUS_SUCCESS) {
        EV_LOGGING(NDI, NOTICE, "NDI-QOS",
                      "npu_id %d priority_group 0x%016lx get failed\n",
                      ndi_port_id.npu_id, ndi_priority_group_id);
        return ndi_utl_mk_qos_std_err(sai_ret);
    }

    // convert sai result to NAS format
    _fill_ndi_qos_priority_group_struct(&attr_list[0], attr_list.size(), p);


    return STD_ERR_OK;

}

/**
 * This function gets the total number of priority_groups on a port
 * @param ndi_port_id
 * @Return standard error code
 */
uint_t ndi_qos_get_number_of_priority_groups(ndi_port_t ndi_port_id)
{
    return ndi_qos_get_priority_group_id_list(ndi_port_id, 1, NULL);
}

/**
 * This function gets the list of priority_groups of a port
 * @param ndi_port_id
 * @param count size of the priority_group_list
 * @param[out] ndi_priority_group_id_list[] to be filled with either the number of priority_groups
 *            that the port owns or the size of array itself, whichever is less.
 * @Return Number of priority_groups that the port owns.
 */
uint_t ndi_qos_get_priority_group_id_list(ndi_port_t ndi_port_id,
                                uint_t count,
                                ndi_obj_id_t *ndi_priority_group_id_list)
{
    /* get priority_group list */
    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(ndi_port_id.npu_id);
    if (ndi_db_ptr == NULL) {
        EV_LOGGING(NDI, DEBUG, "NDI-QOS",
                      "npu_id %d not exist\n", ndi_port_id.npu_id);

        return 0;
    }

    sai_attribute_t sai_attr;
    std::vector<sai_object_id_t> sai_priority_group_id_list(count);

    sai_attr.id = SAI_PORT_ATTR_INGRESS_PRIORITY_GROUP_LIST;
    sai_attr.value.objlist.count = count;
    sai_attr.value.objlist.list = &sai_priority_group_id_list[0];

    sai_object_id_t sai_port;
    if (ndi_sai_port_id_get(ndi_port_id.npu_id, ndi_port_id.npu_port, &sai_port) != STD_ERR_OK) {
        return 0;
    }

    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    sai_ret = ndi_sai_qos_port_api(ndi_db_ptr)->
                        get_port_attribute(sai_port,
                                    1, &sai_attr);

    if (sai_ret != SAI_STATUS_SUCCESS  &&
        sai_ret != SAI_STATUS_BUFFER_OVERFLOW) {
        return 0;
    }

    // copy out sai-returned priority_group ids to nas
    if (ndi_priority_group_id_list) {
        for (uint_t i = 0; (i< sai_attr.value.objlist.count) && (i < count); i++) {
            ndi_priority_group_id_list[i] = sai2ndi_priority_group_id(sai_attr.value.objlist.list[i]);
        }
    }

    return sai_attr.value.objlist.count;

}

static bool nas2sai_priority_group_counter_type_get(BASE_QOS_PRIORITY_GROUP_STAT_t stat_id,
                                                    sai_ingress_priority_group_stat_t * sai_stat_id,
                                                    bool is_snapshot_counters)
{
    static const auto & nas2sai_priority_group_counter_type =
        * new std::unordered_map<BASE_QOS_PRIORITY_GROUP_STAT_t, sai_ingress_priority_group_stat_t, std::hash<int>>
    {
        {BASE_QOS_PRIORITY_GROUP_STAT_PACKETS, SAI_INGRESS_PRIORITY_GROUP_STAT_PACKETS},
        {BASE_QOS_PRIORITY_GROUP_STAT_BYTES, SAI_INGRESS_PRIORITY_GROUP_STAT_BYTES},
        {BASE_QOS_PRIORITY_GROUP_STAT_CURRENT_OCCUPANCY_BYTES, SAI_INGRESS_PRIORITY_GROUP_STAT_CURR_OCCUPANCY_BYTES},
        {BASE_QOS_PRIORITY_GROUP_STAT_WATERMARK_BYTES, SAI_INGRESS_PRIORITY_GROUP_STAT_WATERMARK_BYTES},
        {BASE_QOS_PRIORITY_GROUP_STAT_SHARED_CURRENT_OCCUPANCY_BYTES, SAI_INGRESS_PRIORITY_GROUP_STAT_SHARED_CURR_OCCUPANCY_BYTES},
        {BASE_QOS_PRIORITY_GROUP_STAT_SHARED_WATERMARK_BYTES, SAI_INGRESS_PRIORITY_GROUP_STAT_SHARED_WATERMARK_BYTES},
        {BASE_QOS_PRIORITY_GROUP_STAT_XOFF_ROOM_CURRENT_OCCUPANCY_BYTES, SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_CURR_OCCUPANCY_BYTES},
        {BASE_QOS_PRIORITY_GROUP_STAT_XOFF_ROOM_WATERMARK_BYTES, SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_WATERMARK_BYTES},
    };

    static const auto & nas2sai_priority_group_snapshot_counter_type =
        * new std::unordered_map<BASE_QOS_PRIORITY_GROUP_STAT_t, sai_ingress_priority_group_stat_t, std::hash<int>>
    {
        {BASE_QOS_PRIORITY_GROUP_STAT_CURRENT_OCCUPANCY_BYTES, SAI_INGRESS_PRIORITY_GROUP_STAT_EXTENSIONS_SNAPSHOT_CURR_OCCUPANCY_BYTES},
        {BASE_QOS_PRIORITY_GROUP_STAT_WATERMARK_BYTES, SAI_INGRESS_PRIORITY_GROUP_STAT_EXTENSIONS_SNAPSHOT_WATERMARK_BYTES},
        {BASE_QOS_PRIORITY_GROUP_STAT_SHARED_CURRENT_OCCUPANCY_BYTES, SAI_INGRESS_PRIORITY_GROUP_STAT_EXTENSIONS_SNAPSHOT_SHARED_CURR_OCCUPANCY_BYTES},
        {BASE_QOS_PRIORITY_GROUP_STAT_SHARED_WATERMARK_BYTES, SAI_INGRESS_PRIORITY_GROUP_STAT_EXTENSIONS_SNAPSHOT_SHARED_WATERMARK_BYTES},
        {BASE_QOS_PRIORITY_GROUP_STAT_XOFF_ROOM_CURRENT_OCCUPANCY_BYTES, SAI_INGRESS_PRIORITY_GROUP_STAT_EXTENSIONS_SNAPSHOT_XOFF_ROOM_CURR_OCCUPANCY_BYTES},
        {BASE_QOS_PRIORITY_GROUP_STAT_XOFF_ROOM_WATERMARK_BYTES, SAI_INGRESS_PRIORITY_GROUP_STAT_EXTENSIONS_SNAPSHOT_XOFF_ROOM_WATERMARK_BYTES},
    };


    try {
         if (is_snapshot_counters == false)
             *sai_stat_id = nas2sai_priority_group_counter_type.at(stat_id);
         else
             *sai_stat_id = nas2sai_priority_group_snapshot_counter_type.at(stat_id);
    }
    catch (...) {
        EV_LOGGING(NDI, NOTICE, "NDI-QOS",
                "stats not mapped: stat_id %u\n",
                stat_id);
        return false;
    }
    return true;
}

static void _fill_counter_stat_by_type(sai_ingress_priority_group_stat_t type, uint64_t val,
        nas_qos_priority_group_stat_counter_t *stat )
{
    switch(type) {
    case SAI_INGRESS_PRIORITY_GROUP_STAT_PACKETS:
        stat->packets = val;
        break;
    case SAI_INGRESS_PRIORITY_GROUP_STAT_BYTES:
        stat->bytes = val;
        break;
    case SAI_INGRESS_PRIORITY_GROUP_STAT_CURR_OCCUPANCY_BYTES:
    case SAI_INGRESS_PRIORITY_GROUP_STAT_EXTENSIONS_SNAPSHOT_CURR_OCCUPANCY_BYTES:
        stat->current_occupancy_bytes = val;
        break;
    case SAI_INGRESS_PRIORITY_GROUP_STAT_WATERMARK_BYTES:
    case SAI_INGRESS_PRIORITY_GROUP_STAT_EXTENSIONS_SNAPSHOT_WATERMARK_BYTES:
        stat->watermark_bytes = val;
        break;
    case SAI_INGRESS_PRIORITY_GROUP_STAT_SHARED_CURR_OCCUPANCY_BYTES:
    case SAI_INGRESS_PRIORITY_GROUP_STAT_EXTENSIONS_SNAPSHOT_SHARED_CURR_OCCUPANCY_BYTES:
        stat->shared_current_occupancy_bytes = val;
        break;
    case SAI_INGRESS_PRIORITY_GROUP_STAT_SHARED_WATERMARK_BYTES:
    case SAI_INGRESS_PRIORITY_GROUP_STAT_EXTENSIONS_SNAPSHOT_SHARED_WATERMARK_BYTES:
        stat->shared_watermark_bytes = val;
        break;
    case SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_CURR_OCCUPANCY_BYTES:
    case SAI_INGRESS_PRIORITY_GROUP_STAT_EXTENSIONS_SNAPSHOT_XOFF_ROOM_CURR_OCCUPANCY_BYTES:
        stat->xoff_room_current_occupancy_bytes = val;
        break;
    case SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_WATERMARK_BYTES:
    case SAI_INGRESS_PRIORITY_GROUP_STAT_EXTENSIONS_SNAPSHOT_XOFF_ROOM_WATERMARK_BYTES:
        stat->xoff_room_watermark_bytes = val;
        break;

    default:
        break;
    }
}

/**
 * This function gets the priority_group statistics
 * @param ndi_port_id
 * @param ndi_priority_group_id
 * @param list of priority_group counter types to query
 * @param number of priority_group counter types specified
 * @param[out] counter stats
 * return standard error
 * @deprecated since 7.7.0+opx1
 * @see ndi_qos_get_extended_priority_group_statistics()*
 */
t_std_error ndi_qos_get_priority_group_stats(ndi_port_t ndi_port_id,
                                ndi_obj_id_t ndi_priority_group_id,
                                BASE_QOS_PRIORITY_GROUP_STAT_t *counter_ids,
                                uint_t number_of_counters,
                                nas_qos_priority_group_stat_counter_t *stats)
{
    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(ndi_port_id.npu_id);
    if (ndi_db_ptr == NULL) {
        EV_LOGGING(NDI, DEBUG, "NDI-QOS",
                      "npu_id %d not exist\n", ndi_port_id.npu_id);
        return STD_ERR(QOS, CFG, 0);
    }

    std::vector<sai_ingress_priority_group_stat_t> counter_id_list;
    std::vector<uint64_t> counters(number_of_counters);

    for (uint_t i= 0; i<number_of_counters; i++) {
        sai_ingress_priority_group_stat_t sai_stat_id;
        if (nas2sai_priority_group_counter_type_get(counter_ids[i], &sai_stat_id, false) == true)
            counter_id_list.push_back(sai_stat_id);
    }
    if ((sai_ret = ndi_sai_qos_buffer_api(ndi_db_ptr)->
                        get_ingress_priority_group_stats(ndi2sai_priority_group_id(ndi_priority_group_id),
                                counter_id_list.size(),
                                &counter_id_list[0],
                                &counters[0]))
                         != SAI_STATUS_SUCCESS) {
        EV_LOGGING(NDI, NOTICE, "NDI-QOS",
                "priority_group get stats fails: npu_id %u\n",
                ndi_port_id.npu_id);
        return ndi_utl_mk_qos_std_err(sai_ret);
    }

    // copy the stats out
    for (uint i= 0; i<counter_id_list.size(); i++) {
        _fill_counter_stat_by_type(counter_id_list[i], counters[i], stats);
    }

    return STD_ERR_OK;
}

/**
 * This function gets the priority_group statistics
 * @param ndi_port_id
 * @param ndi_priority_group_id
 * @param list of priority_group counter types to query
 * @param number of priority_group counter types specified
 * @param[out] counter stats
 * @param is counter read and clear
 * @param is snapshot counters
 * return standard error
 */
t_std_error ndi_qos_get_extended_priority_group_statistics(ndi_port_t ndi_port_id,
                                ndi_obj_id_t ndi_priority_group_id,
                                BASE_QOS_PRIORITY_GROUP_STAT_t *counter_ids,
                                uint_t number_of_counters,
                                uint64_t *counters,
                                ndi_stats_mode_t ndi_stats_mode,
                                bool is_snapshot_counters)
{
    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    sai_ingress_priority_group_stat_t sai_stat_id;
    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(ndi_port_id.npu_id);
    if (ndi_db_ptr == NULL) {
        EV_LOGGING(NDI, DEBUG, "NDI-QOS",
                      "npu_id %d not exist\n", ndi_port_id.npu_id);
        return STD_ERR(QOS, CFG, 0);
    }

    std::vector<sai_ingress_priority_group_stat_t> sai_counter_id_list;

    for (uint_t i= 0; i<number_of_counters; i++) {
        if (nas2sai_priority_group_counter_type_get(counter_ids[i], &sai_stat_id,
                                                    is_snapshot_counters) == true)
            sai_counter_id_list.push_back(sai_stat_id);
    }

    std::vector<uint64_t> sai_counters(sai_counter_id_list.size());
    sai_stats_mode_t sai_mode;
    if (ndi_to_sai_stats_mode(ndi_stats_mode, &sai_mode) == false)
        return STD_ERR(QOS, PARAM, 0);

    if ((sai_ret = ndi_sai_qos_buffer_api(ndi_db_ptr)->
                        get_ingress_priority_group_stats_ext(ndi2sai_priority_group_id(ndi_priority_group_id),
                                sai_counter_id_list.size(),
                                &sai_counter_id_list[0],
                                sai_mode,
                                &sai_counters[0]))
                         != SAI_STATUS_SUCCESS) {
        EV_LOGGING(NDI, NOTICE, "NDI-QOS",
                "priority_group get stats fails: npu_id %u\n",
                ndi_port_id.npu_id);
        return ndi_utl_mk_qos_std_err(sai_ret);
    }

    uint_t i, j;
    for (i= 0, j= 0; i < number_of_counters; i++) {
        if (nas2sai_priority_group_counter_type_get(counter_ids[i], &sai_stat_id,
                                           is_snapshot_counters)) {
            counters[i] = sai_counters[j];
            j++;
        }
        else {
            // zero-filled for counters not able to poll
            counters[i] = 0;
        }
    }

    return STD_ERR_OK;
}

/**
 * This function clears the priority_group statistics
 * @param ndi_port_id
 * @param ndi_priority_group_id
 * @param list of priority_group counter types to clear
 * @param number of priority_group counter types specified
 * return standard error
 * @deprecated since 7.7.0+opx1
 * @see ndi_qos_clear_extended_priority_group_statistics()*
 */
t_std_error ndi_qos_clear_priority_group_stats(ndi_port_t ndi_port_id,
                                ndi_obj_id_t ndi_priority_group_id,
                                BASE_QOS_PRIORITY_GROUP_STAT_t *counter_ids,
                                uint_t number_of_counters)
{
    return ndi_qos_clear_extended_priority_group_statistics (ndi_port_id,
                             ndi_priority_group_id, counter_ids,
                             number_of_counters, false);
}

/**
 * This function clears the priority_group statistics
 * @param ndi_port_id
 * @param ndi_priority_group_id
 * @param list of priority_group counter types to clear
 * @param number of priority_group counter types specified
 * @param is snapshot counters
 * return standard error
 */
t_std_error ndi_qos_clear_extended_priority_group_statistics(ndi_port_t ndi_port_id,
                                ndi_obj_id_t ndi_priority_group_id,
                                BASE_QOS_PRIORITY_GROUP_STAT_t *counter_ids,
                                uint_t number_of_counters,
                                bool is_snapshot_counters)
{
    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(ndi_port_id.npu_id);
    if (ndi_db_ptr == NULL) {
        EV_LOGGING(NDI, DEBUG, "NDI-QOS",
                      "npu_id %d not exist\n", ndi_port_id.npu_id);
        return STD_ERR(QOS, CFG, 0);
    }

    std::vector<sai_ingress_priority_group_stat_t> sai_counter_id_list;

    for (uint_t i= 0; i<number_of_counters; i++) {
        sai_ingress_priority_group_stat_t sai_stat_id;
        if (nas2sai_priority_group_counter_type_get(counter_ids[i], &sai_stat_id,
                                                    is_snapshot_counters) == true)
            sai_counter_id_list.push_back(sai_stat_id);
    }

    if (sai_counter_id_list.size() == 0) {
        EV_LOGGING(NDI, DEBUG, "NDI-QOS", "no valid counter id \n");
        return STD_ERR_OK;
    }

    if ((sai_ret = ndi_sai_qos_buffer_api(ndi_db_ptr)->
                        clear_ingress_priority_group_stats(ndi2sai_priority_group_id(ndi_priority_group_id),
                                sai_counter_id_list.size(), &sai_counter_id_list[0]))
                         != SAI_STATUS_SUCCESS) {
        EV_LOGGING(NDI, NOTICE, "NDI-QOS",
                "priority_group clear stats fails: npu_id %u\n",
                ndi_port_id.npu_id);
        return ndi_utl_mk_qos_std_err(sai_ret);
    }

    return STD_ERR_OK;
}

/**
 * This function gets the list of shadow priority group object on different MMUs
 * @param npu_id
 * @param ndi_pg_id
 * @param count, size of ndi_shadow_pg_list[]
 * @param[out] ndi_shadow_pg_list[] will be filled if successful
 * @Return The total number of shadow pg objects on different MMUs.
 *         If the count is smaller than the actual number of shadow pg
 *         objects, ndi_shadow_pg_list[] will not be filled.
 */
uint_t ndi_qos_get_shadow_priority_group_list(npu_id_t npu_id,
                            ndi_obj_id_t ndi_pg_id,
                            uint_t count,
                            ndi_obj_id_t * ndi_shadow_pg_list)
{
    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr;
    std::vector<sai_object_id_t> shadow_pg_list(count);

    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(npu_id);
    if (ndi_db_ptr == NULL) {
        EV_LOGGING(NDI, DEBUG, "NDI-QOS",
                      "npu_id %d not exist\n", npu_id);
        return 0;
    }

    sai_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_SHADOW_PG_LIST;
    sai_attr.value.objlist.count = count;
    sai_attr.value.objlist.list = &(shadow_pg_list[0]);

    if ((sai_ret = ndi_sai_qos_buffer_api(ndi_db_ptr)->
                        get_ingress_priority_group_attribute(
                                ndi2sai_priority_group_id(ndi_pg_id),
                                1, &sai_attr))
                         != SAI_STATUS_SUCCESS) {
        EV_LOGGING(NDI, DEBUG, "NDI-QOS",
                "shadow PG object get: npu_id %u, ndi_pg_id 0x%016lx, rc %d, count %d\n",
                npu_id, ndi_pg_id, sai_ret, sai_attr.value.objlist.count);

        if (sai_ret == SAI_STATUS_BUFFER_OVERFLOW)
            return sai_attr.value.objlist.count;
        else
            return 0;
    }

    for (uint i= 0; i< sai_attr.value.objlist.count; i++) {
        ndi_shadow_pg_list[i] = sai2ndi_priority_group_id(shadow_pg_list[i]);
    }

    EV_LOGGING(NDI, DEBUG, "NDI-QOS-PG",
                  "getting shadow PG count %d\n", sai_attr.value.objlist.count);

    return sai_attr.value.objlist.count;

}

