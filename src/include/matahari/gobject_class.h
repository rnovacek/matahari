/*
 * mh_gobject_class.h
 *
 * Copyright (C) 2010 Red Hat, Inc.
 * Written by Roman Rakus <rrakus@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef MHGOBJECTCLASS_H_
#define MHGOBJECTCLASS_H_

#include "matahari/matahari.h"

#define GMATAHARI_TYPE            (gmatahari_get_type())
#define GMATAHARI(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), \
                                  GMATAHARI_TYPE, GMatahari))
#define GMATAHARI_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                  GMATAHARI_TYPE, GMatahariClass))
#define IS_GMATAHARI(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), \
                                  GMATAHARI_TYPE))
#define IS_GMATAHARI_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                                  GMATAHARI_TYPE))
#define GMATAHARI_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                  GMATAHARI_TYPE, GMatahariClass))

typedef struct _GMatahari GMatahari;
typedef struct _GMatahariClass GMatahariClass;
typedef struct _GMatahariPrivate GMatahariPrivate;

GType
gmatahari_get_type(void);

struct _GMatahari
{
    GObject parent;
    Matahari *matahari;
};

struct _GMatahariClass
{
    GObjectClass parent;
};

#endif /* MHGOBJECTCLASS_H_ */
