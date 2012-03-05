#ifndef __MH_API_SYSCONFIG_LINUX_UNITTEST_H
#define __MH_API_SYSCONFIG_LINUX_UNITTEST_H
#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <utility>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <cxxtest/TestSuite.h>

extern "C" {
#include <glib.h>

#include "sysconfig/sysconfig.h"
#include "matahari/matahari.h"
#include "matahari/matahari-private.h"
#include "matahari/sysconfig_common.h"
#include "matahari/sysconfig_internal.h"
#include "matahari/utilities.h"
#include "mh_test_utilities.h"
};

using namespace std;

/**
 * Callback function, doesn't do anything
 */
void f(void *, enum mh_result res, char *status)
{
    TS_ASSERT(res == MH_RES_SUCCESS);
    TS_ASSERT(status != NULL);
}

/**
 * Result from run_string and run_uri functions is in the form "query = result\n"
 * This function will extract the "result"
 *
 * \note The return of this routine must be freed with free()
 */
char *strip(const char *result)
{
    char *stripped;
   // strip the "query = " part
    stripped = strdup(strstr(result, " = ") + 3);
    // and the line end
    stripped[strlen(stripped) - 1] = '\0';
    return stripped;
}

class MhApiSysconfigLinuxSuite : public CxxTest::TestSuite
{
 public:
    /**
     * This test compares result of mh_sysconfig_query, mh_sysconfig_run_string
     * and mh_sysconfig_run_uri commands. This test is linux-only (no augeas
     * on windows).
     */
    void testAugeasQuery(void)
    {
        Matahari *mh = matahari_new();
        mh_sysconfig_init(mh);
        const char key[] = "org.matahariproject.test.unittest.augeas";
        const char query[] = "/files/etc/mtab/1/spec";
        const char command[] = "get /files/etc/mtab/1/spec";
        char tmp_file[] = "matahari-sysconfig-test_XXXXXX";
        char *query_result, *run_string_result, *run_string_result_stripped;
        char *run_uri_result, *run_uri_result_stripped;
        char *uri, *abs_path;
        char *status;
        int fd = mkstemp(tmp_file);
        TS_ASSERT(fd >= 0);

        mh_sysconfig_keys_dir_set("/tmp/matahari-sysconfig-keys/");

        // test query function
        TS_ASSERT(mh_sysconfig_query(mh, query, 0, "augeas", &query_result) == MH_RES_SUCCESS);
        TS_ASSERT(query_result != NULL);
        TS_ASSERT(strlen(query_result) > 0);

        // test run_string function
        mh->callback = (mh_callback) f;
        TS_ASSERT(mh_sysconfig_run_string(mh, command, MH_SYSCONFIG_FLAG_FORCE, "augeas", key, &status) == MH_RES_ASYNC);
        TS_ASSERT(mh_sysconfig_is_configured(mh, key, &run_string_result) == MH_RES_SUCCESS);
        TS_ASSERT(run_string_result != NULL);

        run_string_result_stripped = strip(run_string_result);
        free(run_string_result);

        // test run_uri function
        write(fd, command, strlen(command));
        write(fd, "\n", 1);
        close(fd);

        abs_path = realpath(tmp_file, NULL);
        asprintf(&uri, "file://%s", abs_path);
        TS_ASSERT(mh_sysconfig_run_uri(mh, uri, MH_SYSCONFIG_FLAG_FORCE, "augeas", key, &status) == MH_RES_ASYNC);
        TS_ASSERT(mh_sysconfig_is_configured(mh, key, &run_uri_result) == MH_RES_SUCCESS);
        TS_ASSERT(run_uri_result != NULL);

        run_uri_result_stripped = strip(run_uri_result);
        free(run_uri_result);
        unlink(tmp_file);

        // compare the results of all functions above
        TS_ASSERT(strcmp(query_result, run_string_result_stripped) == 0);
        TS_ASSERT(strcmp(query_result, run_uri_result_stripped) == 0);

        free(query_result);
        free(run_string_result_stripped);
        free(run_uri_result_stripped);
        free(abs_path);
        free(uri);
    }
};

#endif
