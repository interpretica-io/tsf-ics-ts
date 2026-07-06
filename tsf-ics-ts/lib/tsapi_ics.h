/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief Suite helpers
 *
 * Every test here needs the same thing: an agent and an RPC server on
 * it, because tapi_ics drives Modbus (libmodbus) over that RPC server. This makes
 * one.
 *
 * @author Maxim Menshikov <maxim.menshikov@interpretica.io>
 */

#ifndef __TSAPI_ICS_H__
#define __TSAPI_ICS_H__

#include "rcf_rpc.h"
#include "te_errno.h"

#ifdef __cplusplus
extern "C" {
#endif

/** The agent this suite works on. */
#define TSAPI_ICS_TA   "Agt_A"

/** What a test needs to talk to the agent. */
typedef struct tsapi_ics_session {
    /** Agent name. */
    const char *ta;
    /** RPC server on it - what tapi_ics takes. */
    rcf_rpc_server *pco;
} tsapi_ics_session;

/**
 * Open a session: an RPC server on the agent.
 *
 * @param[out] session  Session.
 * @param[in]  name     Name for the RPC server, unique within the test.
 *
 * @return Status code.
 */
extern te_errno tsapi_ics_session_init(tsapi_ics_session *session,
                                       const char *name);

/**
 * Close a session.
 *
 * @param session       Session.
 */
extern void tsapi_ics_session_fini(tsapi_ics_session *session);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* !__TSAPI_ICS_H__ */
