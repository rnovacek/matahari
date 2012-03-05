/* network.c - Copyright (c) 2010 Red Hat, Inc.
 * Written by Adam Stokes <astokes@fedoraproject.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"

#include "matahari/matahari.h"
#include "network_private.h"
#include "matahari/logging.h"
#include <sigar.h>
#include <sigar_format.h>
#include <stdint.h>

#include "network.h"

MH_TRACE_INIT_DATA(mh_network);

/**
 * An opaque type for a network interface
 */
struct mh_network_interface {
    sigar_net_interface_config_t ifconfig;
};

/**
 * Flags regarding the state of a network interface.
 */
enum mh_network_interface_flags {
    MH_NETWORK_IF_UP =   (1 << 0),
    MH_NETWORK_IF_DOWN = (1 << 1),
};

static GList *
query_interface_table(void)
{
    int status;
    uint32_t lpc = 0;
    GList *interfaces = NULL;
    sigar_t* sigar;
    sigar_net_interface_list_t iflist;

    sigar_open(&sigar);

    status = sigar_net_interface_list_get(sigar, &iflist);
    if (status != SIGAR_OK) {
        goto return_cleanup;
    }

    for (lpc = 0; lpc < iflist.number; lpc++) {
        struct mh_network_interface *iface;

        iface = g_malloc0(sizeof(*iface));

        status = sigar_net_interface_config_get(sigar, iflist.data[lpc],
                                                &iface->ifconfig);

        if (status != SIGAR_OK) {
            g_free(iface);
            continue;
        }

        interfaces = g_list_prepend(interfaces, iface);
    }

    sigar_net_interface_list_destroy(sigar, &iflist);

return_cleanup:
    sigar_close(sigar);

    return interfaces;
}

/**
 * Get the name of a network interface
 *
 * \param[in] iface the network interface
 *
 * \return the name for the provided network interface
 */
const char *
mh_network_interface_get_name(const struct mh_network_interface *iface)
{
    return iface->ifconfig.name;
}

/**
 * Get flags for the state of a network interface
 *
 * \param[in] iface the network interface
 *
 * \return mh_network_interface_flags
 */
uint64_t
mh_network_interface_get_flags(const struct mh_network_interface *iface)
{
    uint64_t flags = 0;

    if (iface->ifconfig.flags & SIGAR_IFF_UP) {
        flags = MH_NETWORK_IF_UP;
    } else {
        flags = MH_NETWORK_IF_DOWN;
    }

    return flags;
}

/**
 * Destroy a network interface
 *
 * This destroys an allocated mh_network_interface.  This function should
 * be used in combination with g_list_free_full() on the rest from
 * mh_network_get_interfaces().  For example:
 *
 * \code
 * GList *iface_list;
 *
 * iface_list = mh_network_get_interfaces();
 * ... do stuff with the iface_list ...
 * g_list_free_full(iface_list, mh_network_interface_destroy);
 * \endcode
 */
void
mh_network_interface_destroy(gpointer data)
{
    g_free(data);
}

static int
network_status(const char *iface, uint64_t *flags)
{
    GList *list = NULL;
    GList *plist;
    int res = 1;

    list = query_interface_table();
    for (plist = g_list_first(list); plist; plist = g_list_next(plist)) {
        struct mh_network_interface *mh_iface = plist->data;

        if ((g_str_equal(mh_network_interface_get_name(mh_iface), iface)) == TRUE) {
            *flags = mh_network_interface_get_flags(mh_iface);
            res = 0;
        }
    }
    g_list_free_full(list, mh_network_interface_destroy);

    return res;
}

static int
interface_status(const char *iface)
{
    uint64_t flags = 0;

    if (iface == NULL) {
        return 3;
    }

    network_status(iface, &flags);

    if (flags & MH_NETWORK_IF_UP) {
        return 0;
    }

    return 1; /* Inactive */
}

typedef struct _NetworkPriv {

} NetworkPriv;

void
mh_network_init(Matahari *matahari)
{
    NetworkPriv *priv = malloc(sizeof(NetworkPriv));
    matahari_set_priv(matahari, priv);
}

char *
mh_network_prop_get_uuid(Matahari *matahari)
{
    return strdup(mh_uuid());
}

char *
mh_network_prop_get_hostname(Matahari *matahari)
{
    return strdup(mh_hostname());
}

int64_t
mh_network_prop_get_qmf_gen_no_crash(Matahari *matahari)
{
    return 0;
}

enum mh_result
mh_network_list(Matahari *matahari, GList **list)
{
    *list = query_interface_table();
    return MH_RES_SUCCESS;
}

enum mh_result
mh_network_start(Matahari *matahari, const char *iface, unsigned int *status)
{
    network_os_start(iface);
    *status = interface_status(iface);
    return MH_RES_SUCCESS;
}

enum mh_result
mh_network_stop(Matahari *matahari, const char *iface, unsigned int *status)
{
    network_os_stop(iface);
    *status = interface_status(iface);
    return MH_RES_SUCCESS;
}

enum mh_result
mh_network_restart(Matahari *matahari, const char *iface)
{
    network_os_stop(iface);
    network_os_start(iface);
    return MH_RES_SUCCESS;
}

enum mh_result
mh_network_status(Matahari *matahari, const char *iface, unsigned int *status)
{
    *status = interface_status(iface);
    return MH_RES_SUCCESS;
}

enum mh_result
mh_network_get_ip_address(Matahari *matahari, const char *iface, char **ip)
{
    GList *list = NULL;
    GList *plist;
    char addr_str[SIGAR_INET6_ADDRSTRLEN];

    list = query_interface_table();
    for (plist = g_list_first(list); plist; plist = g_list_next(plist)) {
        struct mh_network_interface *mh_iface = plist->data;

        if ((g_str_equal(mh_network_interface_get_name(mh_iface), iface)) == TRUE) {
            sigar_net_address_to_string(NULL, &mh_iface->ifconfig.address, addr_str);
            *ip = strdup(addr_str);
            break;
        }
    }
    g_list_free_full(list, mh_network_interface_destroy);
    if (*ip) {
        return MH_RES_SUCCESS;
    } else {
        return MH_RES_INVALID_ARGS;
    }
}

enum mh_result
mh_network_get_mac_address(Matahari *matahari, const char * iface, char * *mac)
{
    GList *list = NULL;
    GList *plist;

    list = query_interface_table();
    for (plist = g_list_first(list); plist; plist = g_list_next(plist)) {
        struct mh_network_interface *mh_iface = plist->data;

        if ((g_str_equal(mh_network_interface_get_name(mh_iface), iface)) == TRUE) {
            asprintf(mac, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
                     mh_iface->ifconfig.hwaddr.addr.mac[0],
                     mh_iface->ifconfig.hwaddr.addr.mac[1],
                     mh_iface->ifconfig.hwaddr.addr.mac[2],
                     mh_iface->ifconfig.hwaddr.addr.mac[3],
                     mh_iface->ifconfig.hwaddr.addr.mac[4],
                     mh_iface->ifconfig.hwaddr.addr.mac[5]);
            break;
        }
    }
    g_list_free_full(list, mh_network_interface_destroy);

    if (*mac) {
        return MH_RES_SUCCESS;
    } else {
        return MH_RES_INVALID_ARGS;
    }
}
