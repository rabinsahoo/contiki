#define	_COMMAND_NS_MOP_C_

#include <stdlib.h>
#include "command.h"
#include "commline/cl_stackline_helpers.h"

#if RPL_WITH_NON_STORING

#include "net/rpl/rpl-ns.h"
#include "net/rpl/rpl-route-projection.h"

int get_route_list(FILE *fp, char *buf, int buflen)
{
  rpl_ns_node_t *r;
  char ipstr[128], parent[128];
  int n=0, wr_comma=0;
  uip_ipaddr_t ip;

  for(r = rpl_ns_node_head(); r != NULL;
      r = rpl_ns_node_next(r)) 
  {
    rpl_ns_get_node_global_addr(&ip, r);
    uip_ipaddr_to_str(&ip, ipstr, sizeof(ipstr));
    rpl_ns_get_node_global_addr(&ip, r->parent);
    uip_ipaddr_to_str(&ip, parent, sizeof(parent));
    if(wr_comma) {
      ADD2BUF(fp, ",");
    }
    wr_comma=1;
    ADD2BUF(fp, "{ \"prefix\": \"%s\", \"pref_len\": \"%d\", \"parent\": \"%s\" }\n", 
        ipstr, 128, parent);
    if(n > buflen-100) {
      n += snprintf(buf+n, buflen-n, "[TRUNC]");
      break;
    }
  }
  return n;
}

int cmd_rtsize(uint16_t id, char *buf, int buflen)
{
  return snprintf(buf, buflen, "%d", rpl_ns_num_nodes());
}

int id2ipaddr(uint16_t id, uip_ipaddr_t *ip)
{
  rpl_dag_t *dag;
  rpl_prefix_t *prefix;
  uip_lladdr_t lladdr;

  // get prefix
  dag = rpl_get_any_dag();
  if(!dag) {
    return 1;
  }
  prefix = &dag->prefix_info;

  memset(ip, 0, sizeof(uip_ipaddr_t));

  // Set prefix
  memcpy(ip, &prefix->prefix, (prefix->length + 7) / 8);

  // id to lladdr
  cl_get_id2longaddr(id, (uint8_t*)&lladdr, sizeof(lladdr));

  // set lower 64 bits to lladdr
  uip_ds6_set_addr_iid(ip, &lladdr);
  return 0;
}

#define MAX_RTPRO_KV  5
int cmd_route_projection(uint16_t id, char *buf, int buflen)
{
#define MAX_VIA 10
  uip_ipaddr_t vip[MAX_VIA];
  uip_ipaddr_t tgtip, dstip;
  char tgtstr[128], dststr[128];
  char *key[MAX_RTPRO_KV], *val[MAX_RTPRO_KV];
  int kv_cnt, via_cnt = 0, ret;
  char *tgt = NULL, *vialist = NULL, *ltime = NULL, *dst = NULL;

  printf("ROUTE PROJECTION:\n%s\n", buf);

  kv_cnt  = util_kv_parse(buf, key, val, MAX_RTPRO_KV);
  tgt     = util_kv_get("tgt", key, val, kv_cnt);
  vialist = util_kv_get("via", key, val, kv_cnt);
  ltime   = util_kv_get("lt",  key, val, kv_cnt);
  dst     = util_kv_get("dst", key, val, kv_cnt);

  if(!tgt || !vialist || !dst) {
    return snprintf(buf, buflen, "tgt/via/dst not specified");
  }

  if(id2ipaddr(atoi(tgt), &tgtip)) {
    return snprintf(buf, buflen, "DAG not found");
  }
  uip_ipaddr_to_str(&tgtip, tgtstr, sizeof(tgtstr));
  id2ipaddr(atoi(dst), &dstip);
  uip_ipaddr_to_str(&dstip, dststr, sizeof(dststr));
  
  while(vialist) {
    id2ipaddr(atoi(vialist), &vip[via_cnt]);
    vialist = strchr(vialist, ',');
    if(vialist) *vialist++ = 0;
    via_cnt++;
  }

  ret = project_dao(&dstip, &tgtip, vip, via_cnt, ltime?atoi(ltime):RPL_DEFAULT_LIFETIME);

  return snprintf(buf, buflen, "dst=%s, tgt=%s, via_cnt=%d, ret=%d\n", 
            dststr, tgtstr, via_cnt, ret);
}

#endif