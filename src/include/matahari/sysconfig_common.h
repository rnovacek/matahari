/* sysconfig_common.h - Copyright (C) 2012 Red Hat, Inc.
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

#ifndef __SYSCONFIG_COMMON_H
#define __SYSCONFIG_COMMON_H

/*! Supported FLAGS */
#define MH_SYSCONFIG_FLAG_FORCE    (1 << 0)

/**
 * Set system as configured
 *
 * \param[in] key config item to define
 * \param[in] contents specifies success/fail with error where applicable
 *
 * \return See enum mh_result
 */
enum mh_result
mh_sysconfig_set_configured(const char *key, const char *contents);

char *
mh_sysconfig_get_key(const char *key);

#endif
