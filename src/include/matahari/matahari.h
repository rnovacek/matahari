
#ifndef __MH_MATAHARI_H__
#define __MH_MATAHARI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "matahari/errors.h"

typedef struct Matahari Matahari;

/**
 * Get private data
 *
 * \param matahari pointer Matahari structure
 * \return custom private data
 */
void *
matahari_get_priv(Matahari *matahari);

/**
 * Set private data
 *
 * \param matahari pointer Matahari structure
 * \param priv custom private data
 */
void
matahari_set_priv(Matahari *matahari, void *priv);

/**
 * Set error which will be send from callback to the caller
 *
 * \param matahari pointer Matahari structure
 * \param error
 */
void
matahari_set_error(Matahari* matahari, GError *error);


/**
 * Invoke this function to emit event (signal for DBus) from the agent
 *
 * \param matahari pointer Matahari structure
 * \param name name of the event
 * \param ... arguments for event, MUST be in same order and same type
 *            as in schema.xml
 */
void
matahari_emit_event(Matahari *matahari, const char *name, ...);

void
matahari_update_properties(Matahari *matahari);

typedef void (*mh_callback)(Matahari *, enum mh_result, ...);

mh_callback
matahari_get_callback(Matahari *matahari);

#ifdef __cplusplus
}
#endif

#endif
