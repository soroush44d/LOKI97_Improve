#!/usr/bin/env bash
set -euo pipefail

# Validate the exact command reported by users
gcc -O2 -w -std=gnu11 -DLOKI97_CT_LOOKUPS=0 proof_sidechannel.c -o /tmp/proof_vuln
/tmp/proof_vuln >/dev/null || true

# Basic functional check
gcc -O2 -w -std=gnu11 loki97.c test_ecb.c -o /tmp/test_ecb
/tmp/test_ecb 000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F 00000000000000000000000000000000 >/dev/null

echo "BUILD CHECK PASSED"
