/** @file
 * @brief ICS Group
 *
 * Read a Modbus endpoint's security posture with tapi_ics_audit() and
 * gate on it. The endpoint is taken from the environment (TSF_ICS_HOST
 * / TSF_ICS_PORT / TSF_ICS_UNIT); with none set the test SKIPs. The
 * write probe is off unless TSF_ICS_WRITE=1 (it writes a register's own
 * value back - reversible - and acts on a real device, so authorized
 * use only). The gate fails on a CRITICAL finding, which is a register
 * whose unauthenticated write was accepted.
 *
 * Copyright (C) 2026 Interpretica Unipessoal Lda
 */

#define TE_TEST_NAME    "ics/audit"

#include "te_config.h"
#include "tapi_test.h"
#include "te_string.h"

#include "tapi_cybersec.h"
#include "tapi_ics.h"
#include "tapi_ics_audit.h"
#include "tsapi_ics.h"

int
main(int argc, char **argv)
{
    tsapi_ics_session sess;
    tapi_ics_endpoint ep;
    tapi_ics_audit_policy policy = tapi_ics_default_audit_policy;
    tapi_cybersec_report report;
    te_string verdict = TE_STRING_INIT;
    bool report_ready = false;
    const char *host = getenv("TSF_ICS_HOST");
    const char *port = getenv("TSF_ICS_PORT");
    const char *unit = getenv("TSF_ICS_UNIT");
    const char *write = getenv("TSF_ICS_WRITE");

    TEST_START;

    if (host == NULL || host[0] == '\0')
        TEST_SKIP("No Modbus endpoint: set TSF_ICS_HOST to run this");

    TEST_STEP("Open a session to the agent");
    CHECK_RC(tsapi_ics_session_init(&sess, "pco_ics_audit"));

    memset(&ep, 0, sizeof(ep));
    ep.proto = TAPI_ICS_MODBUS_TCP;
    ep.host = host;
    ep.port = port != NULL && port[0] != '\0' ? atoi(port) : 502;
    ep.unit = unit != NULL && unit[0] != '\0' ? atoi(unit) : 1;
    ep.timeout_ms = 2000;

    policy.attempt_write = (write != NULL && strcmp(write, "1") == 0);
    RING("auditing %s:%d unit %d (write probe %s)", ep.host, ep.port,
         ep.unit, policy.attempt_write ? "ON" : "off");

    TEST_STEP("Read the Modbus posture into a report");
    tapi_cybersec_report_init(&report);
    report_ready = true;
    CHECK_RC(tapi_ics_audit(sess.pco, &ep, &policy, &report));
    tapi_cybersec_report_log(&report);

    TEST_STEP("The report is well-formed: at least one finding");
    if (tapi_cybersec_report_count(&report, TAPI_CYBERSEC_SEV_INFO) == 0)
        TEST_VERDICT("the ICS audit produced no findings at all");

    TEST_STEP("Gate: fail on CRITICAL (an unauthenticated write accepted)");
    if (tapi_cybersec_report_verdict(&report, TAPI_CYBERSEC_SEV_CRITICAL,
                                     &verdict))
    {
        TEST_VERDICT("%s", verdict.ptr);
    }

    TEST_SUCCESS;

cleanup:
    te_string_free(&verdict);
    if (report_ready)
        tapi_cybersec_report_free(&report);
    tsapi_ics_session_fini(&sess);
    TEST_END;
}
