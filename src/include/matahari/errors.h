/* errors.h - Copyright (C) 2011 Red Hat, Inc.
 *
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

/**
 * \file
 * \brief Error API
 * \ingroup coreapi
 */

#ifndef __MH_ERRORS_H__
#define __MH_ERRORS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>

/**
 * Global enumeration with result codes.
 */
enum mh_result {
    MH_RES_SUCCESS,
    MH_RES_NOT_IMPLEMENTED,
    MH_RES_INVALID_ARGS,
    MH_RES_DOWNLOAD_ERROR,
    MH_RES_BACKEND_ERROR,
    MH_RES_AUTHENTICATION_ERROR,
    MH_RES_OTHER_ERROR,
    MH_RES_ASYNC
};

/**
 * Get string associated with result code.
 *
 * \param[in] res result code, see enum mh_result
 *
 * \return string describing the result
 */
const char *
mh_result_to_str(enum mh_result res);

/**
 * Matahari error
 */

// Matahari error codes
#define MATAHARI_ERROR (matahari_error_quark())

GQuark
matahari_error_quark(void);

typedef GError MatahariError;

#define mh_error_new(code, fmt, args...) g_error_new(MATAHARI_ERROR, code, fmt, args)

#ifdef __cplusplus
}
#endif

#endif
