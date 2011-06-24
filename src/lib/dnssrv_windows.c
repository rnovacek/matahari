/* dnssrv_windows.c - Copyright (c) 2011 Red Hat, Inc.
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

#include "matahari/dnssrv.h"

/**
 * Domain lookup providing a Matahari broker
 *
 * \param[in] srv record query i.e. "_matahari._tcp.matahariproject.org
 * \param[in] set buffer to hold domain retrieved
 * \param[in] set buffer length 
 *
 * \return 0 or greater for successful match
 */

int
mh_srv_lookup(const char *query, char *target, size_t len)
{
    return -1;
}