#include <stdio.h>
#include <string.h>

#include "loki97.c"

typedef struct {
    unsigned int sa_first_col[8];
    unsigned int sb_second_col[8];
} f_trace;

static f_trace trace_f_indices(ULONG64 A, ULONG64 B) {
    f_trace t;
    ULONG64 d, e;

    d.l = ((A.l & ~B.r) | (A.r & B.r));
    d.r = ((A.r & ~B.r) | (A.l & B.r));

    t.sa_first_col[0] = (unsigned int)((d.l >> 24 | d.r << 8) & 0x1FFF);
    t.sa_first_col[1] = (unsigned int)((d.l >> 16) & 0x7FF);
    t.sa_first_col[2] = (unsigned int)((d.l >> 8) & 0x1FFF);
    t.sa_first_col[3] = (unsigned int)(d.l & 0x7FF);
    t.sa_first_col[4] = (unsigned int)((d.r >> 24 | d.l << 8) & 0x7FF);
    t.sa_first_col[5] = (unsigned int)((d.r >> 16) & 0x1FFF);
    t.sa_first_col[6] = (unsigned int)((d.r >> 8) & 0x7FF);
    t.sa_first_col[7] = (unsigned int)(d.r & 0x1FFF);

    {
        unsigned int s;
        s = S1[t.sa_first_col[0]]; e.l = P[s].l >> 7; e.r = P[s].r >> 7;
        s = S2[t.sa_first_col[1]]; e.l |= P[s].l >> 6; e.r |= P[s].r >> 6;
        s = S1[t.sa_first_col[2]]; e.l |= P[s].l >> 5; e.r |= P[s].r >> 5;
        s = S2[t.sa_first_col[3]]; e.l |= P[s].l >> 4; e.r |= P[s].r >> 4;
        s = S2[t.sa_first_col[4]]; e.l |= P[s].l >> 3; e.r |= P[s].r >> 3;
        s = S1[t.sa_first_col[5]]; e.l |= P[s].l >> 2; e.r |= P[s].r >> 2;
        s = S2[t.sa_first_col[6]]; e.l |= P[s].l >> 1; e.r |= P[s].r >> 1;
        s = S1[t.sa_first_col[7]]; e.l |= P[s].l;      e.r |= P[s].r;
    }

    t.sb_second_col[0] = (unsigned int)(((e.l >> 24) & 0xFF) | ((B.l >> 21) & 0x700));
    t.sb_second_col[1] = (unsigned int)(((e.l >> 16) & 0xFF) | ((B.l >> 18) & 0x700));
    t.sb_second_col[2] = (unsigned int)(((e.l >> 8) & 0xFF) | ((B.l >> 13) & 0x1F00));
    t.sb_second_col[3] = (unsigned int)(((e.l) & 0xFF) | ((B.l >> 8) & 0x1F00));
    t.sb_second_col[4] = (unsigned int)(((e.r >> 24) & 0xFF) | ((B.l >> 5) & 0x700));
    t.sb_second_col[5] = (unsigned int)(((e.r >> 16) & 0xFF) | ((B.l >> 2) & 0x700));
    t.sb_second_col[6] = (unsigned int)(((e.r >> 8) & 0xFF) | ((B.l << 3) & 0x1F00));
    t.sb_second_col[7] = (unsigned int)(((e.r) & 0xFF) | ((B.l << 8) & 0x1F00));

    return t;
}

static int count_differences(const unsigned int* a, const unsigned int* b, int n) {
    int diff = 0;
    for (int i = 0; i < n; ++i) if (a[i] != b[i]) diff++;
    return diff;
}

int main(void) {
    const char* hexkey = "000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F";
    const char* hexiv = "00000000000000000000000000000000";

    keyInstance enc_key;
    cipherInstance cipher;

    BYTE p1[BLOCK_SIZE] = {0};
    BYTE p2[BLOCK_SIZE] = {0};
    p2[15] = 0x80;

    if (cipherInit(&cipher, MODE_ECB, (char*)hexiv) != TRUE) return 1;
    if (makeKey(&enc_key, DIR_ENCRYPT, 256, (char*)hexkey) != TRUE) return 1;

    ULONG64 R1 = byteToULONG64(p1 + 8);
    ULONG64 R2 = byteToULONG64(p2 + 8);
    ULONG64 nR1 = add64(R1, enc_key.SK[0]);
    ULONG64 nR2 = add64(R2, enc_key.SK[0]);

    f_trace t1 = trace_f_indices(nR1, enc_key.SK[1]);
    f_trace t2 = trace_f_indices(nR2, enc_key.SK[1]);

    int d1 = count_differences(t1.sa_first_col, t2.sa_first_col, 8);
    int d2 = count_differences(t1.sb_second_col, t2.sb_second_col, 8);

    printf("LOKI97_CT_LOOKUPS=%d\n", LOKI97_CT_LOOKUPS);
    printf("Different S-box indices in Sa column: %d of 8\n", d1);
    printf("Different S-box indices in Sb column: %d of 8\n", d2);

    printf("Sa indices for plaintext-1: ");
    for (int i = 0; i < 8; ++i) printf("%u ", t1.sa_first_col[i]);
    printf("\nSa indices for plaintext-2: ");
    for (int i = 0; i < 8; ++i) printf("%u ", t2.sa_first_col[i]);
    printf("\n");

    if (!LOKI97_CT_LOOKUPS && (d1 > 0 || d2 > 0)) {
        printf("RESULT: Table indices are plaintext/key-dependent -> cache timing side-channel surface exists.\n");
        return 2;
    }

    if (LOKI97_CT_LOOKUPS) {
        printf("RESULT: Build uses constant-time full-table scan lookups; address trace no longer depends on secret index.\n");
        return 0;
    }

    printf("RESULT: No differing table indices found in this sample.\n");
    return 1;
}
