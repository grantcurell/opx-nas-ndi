#
# Copyright (c) 2019 Dell Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
# LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
# FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
#
# See the Apache Version 2.0 License for specific language governing
# permissions and limitations under the License.
#

ACLOCAL_AMFLAGS=-I m4
SUBDIRS = inc
lib_LTLIBRARIES=libopx_nas_ndi.la

libopx_nas_ndi_la_SOURCES= src/hal_shell.c  src/nas_ndi_init.c \
           src/nas_ndi_port.c src/nas_ndi_qos_priority_group.cpp  src/nas_ndi_stg.cpp \
           src/nas_ndi_acl.cpp src/nas_ndi_lag.cpp src/nas_ndi_port_map.cpp  \
           src/nas_ndi_qos_queue.cpp  src/nas_ndi_switch.cpp \
           src/nas_ndi_mac.cpp src/nas_ndi_port_utils.cpp \
           src/nas_ndi_qos_scheduler.cpp  src/nas_ndi_sw_profile.cpp \
           src/nas_ndi_acl_utl.cpp  src/nas_ndi_mac_utl.cpp  src/nas_ndi_qos_buffer_pool.cpp \
           src/nas_ndi_qos_scheduler_group.cpp  src/nas_ndi_udf.cpp \
           src/nas_ndi_fc_init.c src/nas_ndi_map.cpp src/nas_ndi_qos_buffer_profile.cpp \
           src/nas_ndi_qos_wred.cpp src/nas_ndi_udf_utl.cpp \
           src/nas_ndi_fc_map.cpp src/nas_ndi_mirror.cpp src/nas_ndi_qos_map.cpp  \
           src/nas_ndi_route.c src/nas_ndi_utils.cpp \
           src/nas_ndi_fc_stat.cpp  src/nas_ndi_packet.c src/nas_ndi_qos_policer.cpp \
           src/nas_ndi_router_interface.c \
           src/nas_ndi_hash.c src/nas_ndi_qos_port.cpp \
           src/nas_ndi_sflow.cpp \
           src/nas_ndi_obj_cache.cpp \
           src/nas_ndi_qos_port_pool.cpp \
           src/nas_ndi_bridge.cpp \
           src/nas_ndi_l2mc.cpp \
           src/nas_ndi_mcast.cpp \
           src/nas_ndi_qos_wred_ecn.cpp \
           src/nas_ndi_vlan.cpp \
           src/nas_ndi_tunnel.cpp \
           src/nas_ndi_tunnel_map.cpp \
           src/nas_ndi_plat_stat.c \
           src/nas_ndi_ipmc.cpp \
           src/nas_ndi_router_interface_utl.cpp \
           src/nas_ndi_trap.cpp \
           src/nas_ndi_tunnel_obj.cpp

libopx_nas_ndi_la_CPPFLAGS= -D_FILE_OFFSET_BITS=64 -I$(top_srcdir)/inc/opx -I$(includedir)/opx

libopx_nas_ndi_la_CXXFLAGS=-std=c++11

libopx_nas_ndi_la_LDFLAGS=-shared -version-info 1:1:0

libopx_nas_ndi_la_LIBADD=-lpthread -lopx_common -lopx_logging -lopx_nas_common -lsai-0.9.6
