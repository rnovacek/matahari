/*
 * mh_dbus_common.h
 *
 * Copyright (C) 2011 Red Hat, Inc.
 * Written by Radek Novacek <rnovacek@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef MH_DBUS_COMMON_H
#define MH_DBUS_COMMON_H

#include <glib.h>
#include <dbus/dbus-glib.h>

typedef struct DBusMessageIter DBusMessageIter;

/* GObject class definition */
#include "matahari/gobject_class.h"

/* Private struct in Matahari class */
#define GMATAHARI_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GMATAHARI_TYPE, GMatahariPrivate))

/**
 * Check the authorization for given 'action' using PolicyKit. Returns TRUE if
 * user is authorized, otherwise return FALSE and 'error' is set.
 */
gboolean
check_authorization(const gchar *action, GError** error,
                    DBusGMethodInvocation *context);

typedef struct {
    int prop;
    gchar *name, *nick, *desc;
    GParamFlags flags;
    char type;
} Property;

// This array is defined in auto-generated matahari-module-properties.h
extern Property properties[];

/**
 * Start DBus server with name 'bus_name' and object path 'object_path'
 */
int
run_dbus_server(GObject *obj, char *bus_name, char *object_path);

/**
 * Check the authorization for getting the parameter 'name' using PolicyKit
 * action interface.name
 */
gboolean
matahari_get(GMatahari* matahari, const char *interface, const char *name,
             DBusGMethodInvocation *context);

/**
 * Check the authorization for setting the parameter 'name' using PolicyKit
 * action interface.name
 */
gboolean
matahari_set(GMatahari *matahari, const char *interface, const char *name,
             GValue *value, DBusGMethodInvocation *context);

/**
 * This method is used for getting value of DBus property.
 * It must be implemented in each module.
 * Set the value of property_id parameter to parameter value.
 */
void
matahari_get_property(GObject *object, guint property_id, GValue *value,
                      GParamSpec *pspec);

/**
 * This method is used for setting value of DBus property.
 * It must be implemented in each module.
 * New value of property_id is in parameter value.
 */
void
matahari_set_property(GObject *object, guint property_id, const GValue *value,
                      GParamSpec *pspec);

/**
 * Get GParamSpec structure for known types
 *
 * \param name name of the argument
 * \param desc description of the argument
 * \param type name of the type (from schema.xml)
 * \param flags G_PARAM_READABLE or G_PARAM_READWRITE
 *
 * \return GParamSpec object
 */
GParamSpec *
get_paramspec(const char *name, const char *desc, char *type, GParamFlags flags);

/**
 * Implement this class in DBus agent, call g_object_class_install_property
 * there for each property
 */
void
gmatahari_init_properties(GObjectClass *gobject_class);


/**
 * Add GList to DBusMessageIter
 */
void
glist_to_dbus_message_iter(GList *list, DBusMessageIter *iter);

#endif
