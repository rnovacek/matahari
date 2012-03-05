/*
 * mh_dbus_common.c
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

#include "config.h"

#include "matahari/dbus_common.h"
#include "matahari/logging.h"
#include "matahari/mainloop.h"
#include "matahari/errors.h"
#include "matahari/mainloop.h"
#include <glib/gi18n.h>

#include <polkit/polkit.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-glib-bindings.h>

/* Generate the GObject boilerplate */
G_DEFINE_TYPE(GMatahari, gmatahari, G_TYPE_OBJECT)

const DBusGObjectInfo dbus_glib_matahari_object_info;

gboolean
check_authorization(const gchar *action, GError** error,
                    DBusGMethodInvocation *context)
{
    GError *err;
    PolkitAuthorizationResult *result;
    PolkitSubject *subject;
    PolkitAuthority *authority;
    gboolean res;

    if (context == NULL) {
        g_printerr("Context is not set!\n");
        return FALSE;
    }
    err = NULL;
    subject = polkit_system_bus_name_new(dbus_g_method_get_sender(context));
#ifndef HAVE_PK_GET_SYNC
    authority = polkit_authority_get();
    if(!authority) {
        g_printerr("Error in obtaining authority\n");
        return FALSE;
    }
#else
    authority = polkit_authority_get_sync(NULL, &err);
    if (err != NULL) {
        g_printerr("Error in obtaining authority: %s\n", err->message);
        g_set_error(error, MATAHARI_ERROR, MH_RES_AUTHENTICATION_ERROR,
                    "%s", err->message);
        g_error_free(err);
        return FALSE;
    }
#endif
    result = polkit_authority_check_authorization_sync(authority, subject,
            action, NULL,
            POLKIT_CHECK_AUTHORIZATION_FLAGS_ALLOW_USER_INTERACTION, NULL,
            &err);
    if (err != NULL) {
        g_printerr("Error in checking authorization: %s\n", err->message);
        g_set_error(error, MATAHARI_ERROR, MH_RES_AUTHENTICATION_ERROR,
                    "%s", err->message);
        g_error_free(err);
        return FALSE;
    }
    res = polkit_authorization_result_get_is_authorized(result);
    g_object_unref(subject);
    g_object_unref(result);
    g_object_unref(authority);
    if (!res) {
        g_set_error(error, MATAHARI_ERROR, MH_RES_AUTHENTICATION_ERROR,
                    "%s", mh_result_to_str(MH_RES_AUTHENTICATION_ERROR));
        g_printerr("Caller is not authorized for action %s\n", action);
    }
    return res;
}

GParamSpec *
get_paramspec(const char *name, const char *desc, char *type, GParamFlags flags)
{
    if (strcmp(type, "lstr") == 0 || strcmp(type, "sstr") == 0) {
        return g_param_spec_string(name, name, desc, NULL, flags);
    } else if (strcmp(type, "int64") == 0 || strcmp(type, "absTime") == 0) {
        return g_param_spec_int64(name, name, desc, G_MININT64, G_MAXINT64, 0, flags);
    } else if (strncmp(type, "int", 3) == 0) {
        return g_param_spec_int(name, name, desc, G_MININT, G_MAXINT, 0, flags);
    } else if (strcmp(type, "uint64") == 0 || strcmp(type, "deltaTime") == 0) {
        return g_param_spec_uint64(name, name, desc, 0, G_MAXUINT64, 0, flags);
    } else if (strncmp(type, "uint", 4) == 0) {
        return g_param_spec_uint(name, name, desc, 0, G_MAXUINT, 0, flags);
    } else if (strcmp(type, "bool") == 0) {
        return g_param_spec_boolean(name, name, desc, FALSE, flags);
    } else if (strcmp(type, "float") == 0) {
        return g_param_spec_float(name, name, desc, -G_MAXFLOAT, G_MAXFLOAT, 0, flags);
    } else if (strcmp(type, "double") == 0) {
        return g_param_spec_double(name, name, desc, -G_MAXDOUBLE, G_MAXDOUBLE, 0, flags);
    } else if (strcmp(type, "map") == 0) {
        // TODO: fix it
        return g_param_spec_boxed(name, name, desc,
                                  dbus_g_type_get_map("GHashTable",
                                                      G_TYPE_STRING,
                                                      G_TYPE_STRING), flags);
    } else if (strcmp(type, "list") == 0) {
        return g_param_spec_value_array(name, name, desc, g_param_spec_string("", "", "", NULL, flags), flags);
    } else {
        mh_err("Unknown type of property: %s", type);
    }
    return NULL;
}

int
run_dbus_server(GObject *obj, char *bus_name, char *object_path)
{
    GMainLoop* loop = NULL;
    DBusGConnection *connection = NULL;
    GError *error = NULL;
    DBusGProxy *driver_proxy = NULL;
    guint32 request_name_ret;

    // TODO: argument handling

    mh_log_init(bus_name, LOG_DEBUG, TRUE); // TODO: FALSE

    loop = g_main_loop_new(NULL, FALSE);

    // Obtain a connection to the system bus
    connection = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
    if (!connection) {
        g_printerr(_("Failed to open connection to bus: %s\n"), error->message);
        g_error_free(error);
        return 1;
    }

    dbus_g_connection_register_g_object(connection, object_path, obj);

    driver_proxy = dbus_g_proxy_new_for_name(connection, DBUS_SERVICE_DBUS,
                                             DBUS_PATH_DBUS,
                                             DBUS_INTERFACE_DBUS);

    if (!org_freedesktop_DBus_request_name(driver_proxy, bus_name, 0,
                                           &request_name_ret, &error)) {
        g_printerr(_("Failed to get name: %s\n"), error->message);
        g_error_free(error);
        return 1;
    }

    switch (request_name_ret) {
    case DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER:
        break;
    case DBUS_REQUEST_NAME_REPLY_IN_QUEUE:
        g_printerr(_("Looks like another server of this type is already "
                     "running: Reply in queue\n"));
        return 1;
    case DBUS_REQUEST_NAME_REPLY_EXISTS:
        g_printerr(_("Looks like another server of this type is already "
                     "running: Reply exists\n"));
        return 1;
    case DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER:
        g_printerr(_("We are already running\n"));
        return 1;
    default:
        g_printerr(_("Unspecified error\n"));
        return 1;
    }

    mainloop_track_children(G_PRIORITY_DEFAULT);
    g_main_loop_run(loop);
    g_main_loop_unref(loop);
    g_object_unref(driver_proxy);
    return 0;
}

gboolean
matahari_get(GMatahari* matahari, const char *interface, const char *name,
             DBusGMethodInvocation *context)
{
    GError* error = NULL;
    GParamSpec *spec;
    GValue value = {0, };

    char *action = malloc((strlen(interface) + strlen(name) + 2) * sizeof(char));
    sprintf(action, "%s.%s", interface, name);
    if (!check_authorization(action, &error, context)) {
        dbus_g_method_return_error(context, error);
        free(action);
        return FALSE;
    }
    free(action);

    spec = g_object_class_find_property(G_OBJECT_GET_CLASS(matahari), name);
    if (!spec) {
        error = g_error_new(MATAHARI_ERROR, MH_RES_INVALID_ARGS,
                            "%s", mh_result_to_str(MH_RES_INVALID_ARGS));
        dbus_g_method_return_error(context, error);
        g_error_free(error);
        return FALSE;
    }
    g_value_init(&value, spec->value_type);
    g_object_get_property(G_OBJECT(matahari), name, &value);
    dbus_g_method_return(context, &value);
    return TRUE;
}

gboolean
matahari_set(GMatahari *matahari, const char *interface, const char *name,
             GValue *value, DBusGMethodInvocation *context)
{
    GError* error = NULL;
    char *action = malloc((strlen(interface) + strlen(name) + 2) * sizeof(char));
    sprintf(action, "%s.%s", interface, name);
    if (!check_authorization(action, &error, context)) {
        dbus_g_method_return_error(context, error);
        free(action);
        g_error_free(error);
        return FALSE;
    }
    free(action);

    g_object_set_property(G_OBJECT(matahari), name, value);
    dbus_g_method_return(context);
    return TRUE;
}

/* Class init */
static void
gmatahari_class_init(GMatahariClass *matahari_class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(matahari_class);

    gobject_class->set_property = matahari_set_property;
    gobject_class->get_property = matahari_get_property;

    gmatahari_init_properties(gobject_class);

    dbus_g_object_type_install_info(GMATAHARI_TYPE,
                                    &dbus_glib_matahari_object_info);
}

/* Instance init */
static void
gmatahari_init(GMatahari *gmatahari)
{
}

void
glist_to_dbus_message_iter(GList *list, DBusMessageIter *iter)
{
    GList *plist;
    DBusMessageIter subiter;
    dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &subiter);
    for (plist = g_list_first(list); plist; plist = g_list_next(plist)) {
        dbus_message_iter_append_basic(&subiter, DBUS_TYPE_STRING, &plist->data);
    }
    dbus_message_iter_close_container(iter, &subiter);
}
