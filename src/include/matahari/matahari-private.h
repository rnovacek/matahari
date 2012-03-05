
#ifndef __MH_MATAHARI_PRIVATE_H__
#define __MH_MATAHARI_PRIVATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "matahari/matahari.h"

typedef struct Matahari {
    /** Error message */
    GError *error;

    /** Pointer to private data for agent */
    void *priv;

    /** Custom data set by transport layer, DO NOT OVERWRITE! */
    void *data;

    /**
     * Invoke this function when some properties of agent changes
     * \param matahari pointer to this structure
     */
    void (*update_properties_func)(Matahari *matahari);

    /**
     * Invoke this function to emit event (signal for DBus) from the agent
     * \param matahari pointer to this structure
     * \param name name of the event
     * \param ... arguments for event, MUST be in same order and same type
     *            as in schema.xml
     */
    void (*emit_event_func)(Matahari *matahari, const char *name, ...);

    mh_callback callback;
} Matahari;

/**
 * Create empty structure which will be passed to all methods in library.
 * Free with free() function.
 *
 * \return allocated empty structure
 */
Matahari *
matahari_new();


#ifdef __cplusplus
}
#endif

#endif
