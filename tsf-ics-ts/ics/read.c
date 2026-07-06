/** @file
 * @brief ICS Group
 *
 * Read registers and coils from a Modbus TCP endpoint. There is no
 * Modbus device on a stock host, so the endpoint is taken from the
 * environment (TSF_ICS_HOST / TSF_ICS_PORT / TSF_ICS_UNIT) and the test
 * SKIPs cleanly when none is configured. When one is, it reads a run of
 * holding and input registers and of coils/discrete inputs and logs
 * them; a device that simply does not expose an area is not a failure.
 *
 * Copyright (C) 2026 Interpretica Unipessoal Lda
 */

#define TE_TEST_NAME    "ics/read"

#include "te_config.h"
#include "tapi_test.h"
#include "te_string.h"

#include "tapi_ics.h"
#include "tsapi_ics.h"

/** Read up to @p count from @p area and log them; a refusal is tolerated. */
static void
try_area(rcf_rpc_server *pco, const tapi_ics_endpoint *ep,
         tapi_ics_area area, int addr, int count)
{
    int values[16];
    int n = 0;
    te_errno rc;
    int i;
    te_string s = TE_STRING_INIT;

    if (count > (int)TE_ARRAY_LEN(values))
        count = TE_ARRAY_LEN(values);

    rc = tapi_ics_read(pco, ep, area, addr, count, values, &n);
    if (rc != 0)
    {
        RING("%s @%d x%d: not available (%r)", tapi_ics_area2str(area),
             addr, count, rc);
        return;
    }
    for (i = 0; i < n; i++)
        te_string_append(&s, " %d", values[i]);
    RING("%s @%d x%d ->%s", tapi_ics_area2str(area), addr, n,
         te_string_value(&s));
    te_string_free(&s);
}

int
main(int argc, char **argv)
{
    tsapi_ics_session sess;
    tapi_ics_endpoint ep;
    const char *host = getenv("TSF_ICS_HOST");
    const char *port = getenv("TSF_ICS_PORT");
    const char *unit = getenv("TSF_ICS_UNIT");
    te_string id = TE_STRING_INIT;

    TEST_START;

    if (host == NULL || host[0] == '\0')
        TEST_SKIP("No Modbus endpoint: set TSF_ICS_HOST to run this");

    TEST_STEP("Open a session to the agent");
    CHECK_RC(tsapi_ics_session_init(&sess, "pco_ics_read"));

    memset(&ep, 0, sizeof(ep));
    ep.proto = TAPI_ICS_MODBUS_TCP;
    ep.host = host;
    ep.port = port != NULL && port[0] != '\0' ? atoi(port) : 502;
    ep.unit = unit != NULL && unit[0] != '\0' ? atoi(unit) : 1;
    ep.timeout_ms = 2000;
    RING("Modbus endpoint %s:%d unit %d", ep.host, ep.port, ep.unit);

    TEST_STEP("Read a run of each data area (missing areas tolerated)");
    try_area(sess.pco, &ep, TAPI_ICS_HOLDING, 0, 8);
    try_area(sess.pco, &ep, TAPI_ICS_INPUT_REG, 0, 8);
    try_area(sess.pco, &ep, TAPI_ICS_COIL, 0, 8);
    try_area(sess.pco, &ep, TAPI_ICS_DISCRETE, 0, 8);

    TEST_STEP("Ask for a device identification (optional)");
    if (tapi_ics_device_id(sess.pco, &ep, &id) == 0 && id.len != 0)
        RING("device id: %s", te_string_value(&id));
    else
        RING("device id: not provided");
    te_string_free(&id);

    TEST_SUCCESS;

cleanup:
    tsapi_ics_session_fini(&sess);
    TEST_END;
}
