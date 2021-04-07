/**
 * xt_evil - pass evil bits.
 * Copyright (C) 2021 Robert Gray <rpg4231@rit.edu>
 * Copyright (C) 2021 Ben Cartwright-Cox <ben@benjojo.co.uk>
 * Copyright (C) 2013 Changli Gao <xiaosuo@gmail.com>
 *
 *
 * This code is *inside* the kernel and does weird kernel shit
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

//#include "xt_EVIL.h"

#include <linux/module.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/netfilter/x_tables.h>
#include <net/netfilter/ipv4/nf_defrag_ipv4.h>
#include <linux/netfilter.h>
#include <linux/moduleparam.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Robert Gray <rpg4231@rit.edu>");
MODULE_DESCRIPTION("iptables: pass evil bits");
MODULE_ALIAS("ipt_EVIL");

static int __net_init iptable_evil_table_init(struct net *net);

static const struct xt_table packet_evil = {
        .name           = "evil",
        .valid_hooks    = (1<<NF_INET_LOCAL_IN),                // we only care about what's coming in
        .me             = THIS_MODULE,
        .af             = NFPROTO_IPV4,
        .priority       = NF_IP_PRI_FIRST,                      //literally just INT_MIN so we go first
        .table_init     = iptable_evil_table_init,              //function to initialize
};

static unsigned int
iptable_evil_hook(void *priv, struct sk_buff *skb,
                    const struct nf_hook_state *state)
{

    // so this is where the magic happens
    // skb is our packet buffer, so we look at that and return NF_ACCEPT for our funky evil bit packets
    // or XT_CONTINUE to proceed to other tables
    //TODO: make sure these assumptions are correct
    //get the IP header from skb, check if bit is set
    //IP_DF = 0x4000
    //htons(IP_DF)=0x0040
    //htons(IP_DF)<<1=0x0080
    if((ip_hdr(skb)->frag_off & 0x0040) == 0x0040) {
//    if((ip_hdr(skb)->frag_off & 0x0080) == 0x0080) {
        pr_info("NF_ACCEPT on packet with frag_off: %x",ip_hdr(skb)->frag_off);
        return NF_ACCEPT;
    } else{
        pr_info("XT_RETURN on packet with frag_off: %x",ip_hdr(skb)->frag_off);
        return XT_RETURN; // might need to be XT_RETURN??
    }
    //to set is: iph->frag_off |= htons(IP_DF)<< 1; // Set the "evil" bit
    //return ipt_do_table(skb, state, state->net->ipv4.iptable_filter);
}

static struct nf_hook_ops *evil_ops __read_mostly;

/* Default to forward because I got too much mail already. */
// ^ sic, no idea what's going on there
static bool forward __read_mostly = true;
module_param(forward, bool, 0000);

static int __net_init iptable_evil_table_init(struct net *net)
{
    struct ipt_replace *repl;
    int err;

    //check if we're already loaded
    if (net->ipv4.iptable_evil)
        return 0;

    repl = ipt_alloc_initial_table(&packet_evil);
    if (repl == NULL)
        return -ENOMEM;
    /* Entry 1 is the FORWARD hook */
    ((struct ipt_standard *)repl->entries)[1].target.verdict =
            forward ? -NF_ACCEPT - 1 : -NF_DROP - 1;

    //TODO: create net->ipv4.iptable_evil
    // edit include/net/netns/ipv4.h: netns_ipv4 struct to include iptable_evil and hope it doesn't break it
    // since this modifies kernel headers, this _will_ require building in-tree
    err = ipt_register_table(net, &packet_evil, repl, evil_ops,
                             &net->ipv4.iptable_evil);
    kfree(repl);
    return err;
}

static int __net_init iptable_evil_net_init(struct net *net)
{
    if (net == &init_net || !forward)
        return iptable_evil_table_init(net);

    return 0;
}

static void __net_exit iptable_evil_net_exit(struct net *net)
{
    if (!net->ipv4.iptable_evil)
        return;
    ipt_unregister_table(net, net->ipv4.iptable_evil, evil_ops);
    net->ipv4.iptable_evil = NULL;
}

static struct pernet_operations iptable_evil_net_ops = {
        .init = iptable_evil_net_init,
        .exit = iptable_evil_net_exit,
};

static int __init iptable_evil_init(void)
{
    int ret;

    evil_ops = xt_hook_ops_alloc(&packet_evil, iptable_evil_hook);
    if (IS_ERR(evil_ops))
        return PTR_ERR(evil_ops);

    ret = register_pernet_subsys(&iptable_evil_net_ops);
    if (ret < 0)
        kfree(evil_ops);

    return ret;
}

static void __exit iptable_evil_fini(void)
{
    unregister_pernet_subsys(&iptable_evil_net_ops);
    kfree(evil_ops);
}

module_init(iptable_evil_init);
module_exit(iptable_evil_fini);
