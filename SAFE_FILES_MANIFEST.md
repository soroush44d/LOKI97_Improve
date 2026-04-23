# Safe files manifest (copy-safe-files branch)

اگر می‌خواهی فایل‌های سالم را دستی کپی کنی، فقط این فایل‌ها را از این branch بردار:

- `loki97.c`
- `loki97.h`
- `proof_sidechannel.c`
- `sidechannel_test.c`
- `test_ecb.c`
- `check_build.sh`
- `HOW_TO_OPEN_PR.md`
- `SIDE_CHANNEL_REPORT.md`

## SHA256 (for integrity check)

```text
7cbeb4a99e4ddee7d11f6b32eb66b86c12c6578e2b4d5cf530a645585c32801d  loki97.c
973fbff75726a24dd425c5e93ab338c7e678677b5585dffba2f2c3b87b0d0d07  loki97.h
5e46c368b429182a65398515937f2ec7dae3347631bbe37b470f84abcaa1a9e4  proof_sidechannel.c
840ea222c97f95537007227f10e4d57fc199c3762b4c89f904c8c10523402d74  sidechannel_test.c
94f4f51f9ab4cc86a444602b20bb5c129f26d567e430aef0460dcaca1f55e3a3  test_ecb.c
cfe05784b55dbfbe76bcde6fd271439134879d714ed6467e11c051abfb5fdaf9  check_build.sh
f834341c9ba2f049ddd2a0fe2c978a021f918b0514428a845b4ad0ed8a58e50e  HOW_TO_OPEN_PR.md
fe813f612b396f84753cfeeff662eb766705213cfceafd2385c5cfbbba6bcc25  SIDE_CHANNEL_REPORT.md
```

## Quick verify

```bash
./check_build.sh
```

Expected tail output:

`BUILD CHECK PASSED`
