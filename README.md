# tsf-ics-ts

A Test Environment suite that exercises
[tsf-ics](https://github.com/interpretica-io/tsf-ics) (`tapi_ics`)
against a Modbus TCP endpoint the agent can reach — reading registers
and coils over libmodbus and reading the endpoint's security posture.

| Test | What it checks |
|---|---|
| `read` | reads a run of holding/input registers and coils/discrete inputs from the endpoint, and a device identification; a device that does not expose an area is tolerated, not failed |
| `audit` | `tapi_ics_audit()` produces a well-formed report (readable areas, a device-id fingerprint) and the gate fails on a CRITICAL finding — a register whose unauthenticated write was accepted |

## Authorized use only

Modbus has no authentication: a reachable endpoint answers anyone, and
the (opt-in) write probe changes a register — even though it writes the
register's own value straight back. Point this suite only at a device
you own or are engaged to test.

## Configuring the endpoint

There is no Modbus device on a stock host, so **both tests SKIP cleanly
unless an endpoint is configured** through the environment:

```
TSF_ICS_HOST=192.0.2.10   # required; without it the tests skip
TSF_ICS_PORT=502          # optional, default 502
TSF_ICS_UNIT=1            # optional, default 1
TSF_ICS_WRITE=1           # optional; enables the reversible write probe
```

## Running it

Needs Docker and `test-environment` as a sibling directory:

```bash
./scripts/run.sh docker guess --cfg=localhost          # containerised agent
./scripts/run.sh guess --cfg=localhost                 # native agent (host)
```

The agent host must carry **libmodbus with its headers**
(`libmodbus-dev`, already in the suite's Dockerfile). `tapi_ics`'s
posture reports through tsf-cybersec, so the suite also builds the
tsf-cybersec chain (tsf-cybersec → tsf-kernel → tsf-devtool); refs are
in `conf/external.yml`. The Builder clones the tsf-* repositories
itself, so the checkouts beside this suite are not what a run compiles.

## Status

**Not yet run.** Written alongside tsf-ics; not built or executed here
(no TE toolchain). tsf-ics's libmodbus usage was syntax-checked against
the real headers, but the engine C was not compiled and no live Modbus
request was made. The first run should expect the ordinary first-build
fixes. With no endpoint configured the suite is a clean pass (both tests
skip).
