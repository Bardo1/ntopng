/*
 *
 * (C) 2013-19 - ntop.org
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include "ntop_includes.h"

ArpStatsMatrixElement::ArpStatsMatrixElement(NetworkInterface *_iface, const u_int8_t _src_mac[6], const u_int8_t _dst_mac[6], bool * const src2dst): GenericHashEntry(_iface) {
  memcpy(src_mac, _src_mac, 6);
  memcpy(dst_mac, _dst_mac, 6);
  memset(&stats, 0, sizeof(stats));
  *src2dst = true;

#ifdef ARP_STATS_MATRIX_ELEMENT_DEBUG
  char buf1[32], buf2[32];
  ntop->getTrace()->traceEvent(TRACE_NORMAL, "ADDED ArpMatrixElement: SourceMac %s - DestinationMac %s [num_elements: %u]",
			       Utils::formatMac(src_mac, buf1, sizeof(buf1)),
			       Utils::formatMac(dst_mac, buf2, sizeof(buf2)),
			       iface->getNumArpStatsMatrixElements());
#endif
}

/* *************************************** */

ArpStatsMatrixElement::~ArpStatsMatrixElement(){
#ifdef ARP_STATS_MATRIX_ELEMENT_DEBUG
  char buf1[32], buf2[32];
  ntop->getTrace()->traceEvent(TRACE_NORMAL, "DELETED ArpMatrixElement: SourceMac %s - DestinationMac %s [num_elements: %u]",
			       Utils::formatMac(src_mac, buf1, sizeof(buf1)),
			       Utils::formatMac(dst_mac, buf2, sizeof(buf2)),
			       iface->getNumArpStatsMatrixElements());
#endif

}
/* *************************************** */

bool ArpStatsMatrixElement::idle() {
  bool rc;

  if((num_uses > 0) || (!iface->is_purge_idle_interface()))
    return(false);

  rc = isIdle(MAX_LOCAL_HOST_IDLE);

  return(rc);
}

/* *************************************** */

bool ArpStatsMatrixElement::equal(const u_int8_t _src_mac[6], const u_int8_t _dst_mac[6], bool * const src2dst) const {
  if(!_src_mac || !_dst_mac)
    return false;

  if(memcmp(src_mac, _src_mac, 6) == 0 && memcmp(dst_mac, _dst_mac, 6) == 0) {
    *src2dst = true;
    return true;
  } else if(memcmp(src_mac, _dst_mac, 6) == 0 && memcmp(dst_mac, _src_mac, 6) == 0) {
    *src2dst = false;
    return true;
  }

  return false;
}

/* *************************************** */

u_int32_t ArpStatsMatrixElement::key() {
  return Utils::macHash(src_mac) + Utils::macHash(dst_mac);
}

/* *************************************** */

void ArpStatsMatrixElement::print() const {
  char buf1[32], buf2[32];
  ntop->getTrace()->traceEvent(TRACE_NORMAL, "[SourceMac: %s][DestinationMac: %s]",
			       Utils::formatMac(src_mac, buf1, sizeof(buf1)),
			       Utils::formatMac(dst_mac, buf2, sizeof(buf2)));
}

/* *************************************** */

void ArpStatsMatrixElement::lua(lua_State* vm) {
  char buf[32];

  lua_newtable(vm); /* Outer table key, source mac */
  lua_newtable(vm); /* Innter table key, destination mac */

  lua_push_uint64_table_entry(vm, "src2dst.requests", stats.src2dst.requests);
  lua_push_uint64_table_entry(vm, "src2dst.replies", stats.src2dst.replies);
  lua_push_uint64_table_entry(vm, "dst2src.requests", stats.dst2src.requests);
  lua_push_uint64_table_entry(vm, "dst2src.replies", stats.dst2src.replies);

  /* Destination Mac as the key of the inner table */
  Utils::formatMac(dst_mac, buf, sizeof(buf));
  lua_pushstring(vm, buf);
  lua_insert(vm, -2);
  lua_settable(vm, -3);

  /* Source Mac as the key of the outer table */
  Utils::formatMac(src_mac, buf, sizeof(buf));
  lua_pushstring(vm, buf);
  lua_insert(vm, -2);
  lua_settable(vm, -3);
}
