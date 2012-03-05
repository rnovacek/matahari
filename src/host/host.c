/* host.c - Copyright (C) 2010 Red Hat, Inc.
 * Written by Andrew Beekhof <andrew@beekhof.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#include "host.h"
#include "matahari/logging.h"
#include "matahari/matahari.h"
#include "host_private.h"

#include <sigar.h>

typedef struct cpuinfo_s {
    char *model;
    unsigned int cpus;
    unsigned int cores;
} cpuinfo_t;

typedef struct _HostPriv {
    unsigned int update_interval;
    sigar_t *sigar;
    gboolean sigar_init;
    struct cpuinfo_s cpuinfo;
    unsigned int sequence;

    const char *operating_system;
    const char *arch;

    char *custom_uuid;
    const char *immutable_uuid;
    const char *hardware_uuid;
    const char *reboot_uuid;
    const char *agent_uuid;
} HostPriv;

#define priv(mh) ((HostPriv *) matahari_get_priv(mh))

static void
host_get_cpu_details(Matahari *matahari);

gboolean
mh_host_heartbeat_timer(void *data)
{
    long timestamp = 0L, now = 0L;
    Matahari *matahari = (Matahari *) data;

#ifdef HAVE_TIME
    timestamp = time(NULL);
#endif
    now = timestamp * 1000000000;

    matahari_emit_event(matahari, "heartbeat", now, priv(matahari)->sequence++,
                        mh_hostname(), mh_uuid());
    mh_debug("Heartbeat timer: %ld %u", now, priv(matahari)->sequence);
    // Restart timer
    g_timeout_add(priv(matahari)->update_interval * 1000,
                  mh_host_heartbeat_timer, matahari);
    return FALSE;
}


void
mh_host_init(Matahari *matahari)
{
    HostPriv *priv = malloc(sizeof(HostPriv));
    priv->update_interval = 5;
    priv->sigar_init = TRUE;
    sigar_open(&(priv->sigar));
    priv->sequence = 0;
    priv->operating_system = NULL;
    priv->arch = NULL;
    priv->custom_uuid = NULL;
    priv->immutable_uuid = NULL;
    priv->hardware_uuid = NULL;
    priv->reboot_uuid = NULL;
    priv->agent_uuid = NULL;
    priv->cpuinfo.model = NULL;
    priv->cpuinfo.cpus = 0;
    priv->cpuinfo.cores = 0;

    matahari_set_priv(matahari, priv);

    mh_host_heartbeat_timer(matahari);
}

char *
mh_host_prop_get_hostname(Matahari *matahari)
{
    return strdup(mh_hostname());
}

char *
mh_host_prop_get_os(Matahari *matahari)
{
    if (priv(matahari)->operating_system == NULL) {
        sigar_sys_info_t sys_info;

        sigar_sys_info_get(priv(matahari)->sigar, &sys_info);
        priv(matahari)->operating_system = g_strdup_printf("%s (%s)",
                sys_info.vendor_name, sys_info.version);
    }

    return strdup(priv(matahari)->operating_system);
}

uint8_t
mh_host_prop_get_wordsize(Matahari *matahari)
{
    return (CHAR_BIT * sizeof(size_t));
}

char *
mh_host_prop_get_arch(Matahari *matahari)
{
    if (priv(matahari)->arch == NULL) {
        sigar_sys_info_t sys_info;

        sigar_sys_info_get(priv(matahari)->sigar, &sys_info);
        priv(matahari)->arch = g_strdup(sys_info.arch);
    }

    return strdup(priv(matahari)->arch);
}

enum mh_result
mh_host_reboot(Matahari *matahari)
{
    host_os_reboot();
    return MH_RES_SUCCESS;
}

enum mh_result
mh_host_shutdown(Matahari *matahari)
{
    host_os_shutdown();
    return MH_RES_SUCCESS;
}

char *
mh_host_prop_get_cpu_model(Matahari *matahari)
{
    host_get_cpu_details(matahari);
    return strdup(priv(matahari)->cpuinfo.model);
}

char *
mh_host_prop_get_cpu_flags(Matahari *matahari)
{
    return strdup(host_os_get_cpu_flags());
}

uint8_t
mh_host_prop_get_cpu_count(Matahari *matahari)
{
    host_get_cpu_details(matahari);
    return priv(matahari)->cpuinfo.cpus;
}

uint8_t
mh_host_prop_get_cpu_cores(Matahari *matahari)
{
    host_get_cpu_details(matahari);
    return priv(matahari)->cpuinfo.cores;
}

GHashTable *
mh_host_prop_get_load(Matahari *matahari)
{
    char *str;
    GHashTable *t = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, free);
    sigar_loadavg_t avg;
    sigar_loadavg_get(priv(matahari)->sigar, &avg);

    // For now all maps are string -> string
    asprintf(&str, "%f", avg.loadavg[0]);
    g_hash_table_insert(t, "1", str);
    asprintf(&str, "%f", avg.loadavg[1]);
    g_hash_table_insert(t, "5", str);
    asprintf(&str, "%f", avg.loadavg[2]);
    g_hash_table_insert(t, "15", str);

    return t;
}

GHashTable *
mh_host_prop_get_process_statistics(Matahari *matahari)
{
    char *str;
    GHashTable *t = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, free);
    sigar_proc_stat_t procs;
    sigar_proc_stat_get(priv(matahari)->sigar, &procs);

    // For now all maps are string -> string
    asprintf(&str, "%lu", procs.total);
    g_hash_table_insert(t, "total",    str);
    asprintf(&str, "%lu", procs.idle);
    g_hash_table_insert(t, "idle",     str);
    asprintf(&str, "%lu", procs.zombie);
    g_hash_table_insert(t, "zombie",   str);
    asprintf(&str, "%lu", procs.running);
    g_hash_table_insert(t, "running",  str);
    asprintf(&str, "%lu", procs.stopped);
    g_hash_table_insert(t, "stopped",  str);
    asprintf(&str, "%lu", procs.sleeping);
    g_hash_table_insert(t, "sleeping", str);

    return t;
}

unsigned int
mh_host_prop_get_sequence(Matahari *matahari)
{
    return priv(matahari)->sequence;
}

uint64_t
mh_host_prop_get_memory(Matahari *matahari)
{
    sigar_mem_t mem;
    uint64_t total;

    sigar_mem_get(priv(matahari)->sigar, &mem);
    total = mem.total / 1024;
    return total;
}

uint64_t
mh_host_prop_get_free_mem(Matahari *matahari)
{
    sigar_mem_t mem;
    uint64_t free_mem;

    sigar_mem_get(priv(matahari)->sigar, &mem);
    free_mem = mem.free / 1024;
    return free_mem;
}

uint64_t
mh_host_prop_get_swap(Matahari *matahari)
{
    sigar_swap_t swap;
    uint64_t total;

    sigar_swap_get(priv(matahari)->sigar, &swap);
    total = swap.total / 1024;
    return total;
}

uint64_t
mh_host_prop_get_free_swap(Matahari *matahari)
{
    sigar_swap_t swap;
    uint64_t free_mem;

    sigar_swap_get(priv(matahari)->sigar, &swap);
    free_mem = swap.free / 1024;
    return free_mem;
}

static void
host_get_cpu_details(Matahari *matahari)
{
    sigar_cpu_info_list_t procs;

    if (priv(matahari)->cpuinfo.cpus) {
        return;
    }

    sigar_cpu_info_list_get(priv(matahari)->sigar, &procs);

    priv(matahari)->cpuinfo.cpus = procs.number;
    if (procs.number) {
        sigar_cpu_info_t *proc = (sigar_cpu_info_t *)procs.data;
        priv(matahari)->cpuinfo.model = g_strdup(proc->model);
        priv(matahari)->cpuinfo.cores = proc->total_cores;
    }

    sigar_cpu_info_list_destroy(priv(matahari)->sigar, &procs);
}

enum mh_result
mh_host_identify(Matahari *matahari)
{
    return (host_os_identify() == 0 ? MH_RES_SUCCESS : MH_RES_BACKEND_ERROR);
}

enum mh_result
mh_host_get_uuid(Matahari *matahari, const char * lifetime, char **uuid)
{
    const char *temp = NULL;
    if (mh_strlen_zero(lifetime) || !strcasecmp("filesystem", lifetime)) {
        if (!priv(matahari)->immutable_uuid) {
            priv(matahari)->immutable_uuid = mh_uuid();
        }
        temp = priv(matahari)->immutable_uuid;
    } else if (!strcasecmp("hardware", lifetime)) {
        if (!priv(matahari)->hardware_uuid) {
            /* Check for a UUID from SMBIOS first. */
            priv(matahari)->hardware_uuid = host_os_machine_uuid();
        }
        if (!priv(matahari)->hardware_uuid) {
            /* If SMBIOS wasn't available, then maybe we're on EC2, try that. */
            priv(matahari)->hardware_uuid = host_os_ec2_instance_id();
        }
        temp = priv(matahari)->hardware_uuid;
    } else if (!strcasecmp("reboot", lifetime)) {
        if (!priv(matahari)->reboot_uuid) {
            priv(matahari)->reboot_uuid = host_os_reboot_uuid();
        }
        temp = priv(matahari)->reboot_uuid;
    } else if (!strcasecmp("agent", lifetime)) {
        if (!priv(matahari)->agent_uuid) {
            priv(matahari)->agent_uuid = host_os_agent_uuid();
        }
        temp = priv(matahari)->agent_uuid;
    } else if (!strcasecmp("custom", lifetime)) {
        if (!priv(matahari)->custom_uuid) {
            priv(matahari)->custom_uuid = host_os_custom_uuid();
        }
        temp = priv(matahari)->custom_uuid;
    } else {
        temp = "invalid-lifetime";
    }

    if (temp == NULL)
        *uuid = strdup("not-available");
    else
        *uuid = strdup(temp);

    return MH_RES_SUCCESS;
}

char *
mh_host_prop_get_uuid(Matahari *matahari)
{
    char *uuid;
    mh_host_get_uuid(matahari, "Filesystem", &uuid);
    return uuid;
}

char *
mh_host_prop_get_custom_uuid(Matahari *matahari)
{
    char *uuid;
    mh_host_get_uuid(matahari, "custom", &uuid);
    return uuid;
}

unsigned int
mh_host_prop_get_update_interval(Matahari *matahari)
{
    return priv(matahari)->update_interval;
}

void
mh_host_prop_set_update_interval(Matahari *matahari,
                                 unsigned int update_interval)
{
    priv(matahari)->update_interval = update_interval;
}

int64_t
mh_host_prop_get_last_updated(Matahari *matahari)
{
    int64_t timestamp = 0L, now = 0L;
#ifdef HAVE_TIME
    timestamp = time(NULL);
#endif

    now = timestamp * 1000000000;
    return now;
}

enum mh_result
mh_host_set_uuid(Matahari *matahari, const char *lifetime, const char *uuid,
                 int *rc)
{
    *rc = G_FILE_ERROR_NOSYS;
    if (!mh_strlen_zero(lifetime) && !strcasecmp("custom", lifetime)) {
        *rc = host_os_set_custom_uuid(uuid);
        free(priv(matahari)->custom_uuid);
        priv(matahari)->custom_uuid = host_os_custom_uuid();

        // propagate change of property to Agent
        matahari_update_properties(matahari);
        return MH_RES_SUCCESS;
    }

    GError *error = mh_error_new(MH_RES_BACKEND_ERROR,
                                 "Unable to set UUID for lifetime: '%s'",
                                 mh_strlen_zero(lifetime) ? "" : lifetime);
    matahari_set_error(matahari, error);
    mh_warn("%s", error->message);

    return MH_RES_BACKEND_ERROR;
}

enum mh_result
mh_host_set_power_profile(Matahari *matahari, const char *profile,
                          unsigned int *status)
{
    *status = 0;
    return host_os_set_power_profile(profile);
}

enum mh_result
mh_host_get_power_profile(Matahari *matahari, char **profile)
{
    return host_os_get_power_profile(profile);
}

enum mh_result
mh_host_list_power_profiles(Matahari *matahari, GList **profiles)
{
    *profiles = host_os_list_power_profiles();
    return MH_RES_SUCCESS;
}
