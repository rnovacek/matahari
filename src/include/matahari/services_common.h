/* services_common.h - Copyright (C) 2012 Red Hat, Inc.
 * Written by Radek Novacek <rnovacek@redhat.com>
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

#ifndef __SERVICES_COMMON_H
#define __SERVICES_COMMON_H


#define SYSTEMCTL "/bin/systemctl"

/* TODO: Autodetect these two in CMakeList.txt */
#define OCF_ROOT "/usr/lib/ocf"
#define LSB_ROOT "/etc/init.d"

enum lsb_exitcode {
    LSB_OK = 0,
    LSB_UNKNOWN_ERROR = 1,
    LSB_INVALID_PARAM = 2,
    LSB_UNIMPLEMENT_FEATURE = 3,
    LSB_INSUFFICIENT_PRIV = 4,
    LSB_NOT_INSTALLED = 5,
    LSB_NOT_CONFIGURED = 6,
    LSB_NOT_RUNNING = 7,

    /* 150-199  reserved for application use */
    LSB_SIGNAL        = 194,
    LSB_NOT_SUPPORTED = 195,
    LSB_PENDING       = 196,
    LSB_CANCELLED     = 197,
    LSB_TIMEOUT       = 198,
    LSB_OTHER_ERROR   = 199,
};

/* The return codes for the status operation are not the same for other
 * operatios - go figure */
enum lsb_status_exitcode {
    LSB_STATUS_OK = 0,
    LSB_STATUS_VAR_PID = 1,
    LSB_STATUS_VAR_LOCK = 2,
    LSB_STATUS_NOT_RUNNING = 3,
    LSB_STATUS_NOT_INSTALLED = 4,

    /* 150-199 reserved for application use */
    LSB_STATUS_SIGNAL        = 194,
    LSB_STATUS_NOT_SUPPORTED = 195,
    LSB_STATUS_PENDING       = 196,
    LSB_STATUS_CANCELLED     = 197,
    LSB_STATUS_TIMEOUT       = 198,
    LSB_STATUS_OTHER_ERROR   = 199,
};

enum ocf_exitcode {
    OCF_OK = 0,
    OCF_UNKNOWN_ERROR = 1,
    OCF_INVALID_PARAM = 2,
    OCF_UNIMPLEMENT_FEATURE = 3,
    OCF_INSUFFICIENT_PRIV = 4,
    OCF_NOT_INSTALLED = 5,
    OCF_NOT_CONFIGURED = 6,
    OCF_NOT_RUNNING = 7,
    OCF_RUNNING_MASTER = 8,
    OCF_FAILED_MASTER = 9,

    /* 150-199  reserved for application use */
    OCF_SIGNAL        = 194,
    OCF_NOT_SUPPORTED = 195,
    OCF_PENDING       = 196,
    OCF_CANCELLED     = 197,
    OCF_TIMEOUT       = 198,
    OCF_OTHER_ERROR   = 199, /* Keep the same codes as LSB */
};

enum op_status {
    LRM_OP_PENDING = -1,
    LRM_OP_DONE,
    LRM_OP_CANCELLED,
    LRM_OP_TIMEOUT,
    LRM_OP_NOTSUPPORTED,
    LRM_OP_ERROR
};

typedef struct svc_action_s svc_action_t;

svc_action_t *
mh_services_action_create_generic(const char *exec, const char *args[]);

svc_action_t *
services_action_create(const char *name, const char *action,
                       int interval /* ms */, int timeout /* ms */);

void
services_action_free(svc_action_t *op);

gboolean
services_action_sync(svc_action_t *op);

gboolean
services_action_async(svc_action_t* op, void (*action_callback)(svc_action_t *));

gboolean
services_action_cancel(const char *name, const char *action, int interval);

GList *
get_directory_list(const char *root, gboolean files);

/**
 * Create a resources action.
 *
 * \param[in] timeout the timeout in milliseconds
 * \param[in] interval how often to repeat this action, in milliseconds.
 *            If this value is 0, only execute this action one time.
 *
 * \post After the call, 'params' is owned, and later free'd by the svc_action_t result
 */
svc_action_t *
resources_action_create(const char *name, const char *standard,
                        const char *provider, const char *agent,
                        const char *action, int interval /* ms */,
                        int timeout /* ms */, GHashTable *params);

#endif
