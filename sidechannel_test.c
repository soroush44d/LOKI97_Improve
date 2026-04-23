#define _POSIX_C_SOURCE 200809L
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "loki97.h"

#ifndef SAMPLES
#define SAMPLES 4000
#endif

static uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static double mean_u64(const uint64_t* v, size_t n) {
    long double sum = 0.0;
    for (size_t i = 0; i < n; ++i) sum += (long double)v[i];
    return (double)(sum / (long double)n);
}

static double var_u64(const uint64_t* v, size_t n, double m) {
    long double acc = 0.0;
    for (size_t i = 0; i < n; ++i) {
        long double d = (long double)v[i] - (long double)m;
        acc += d * d;
    }
    return (double)(acc / (long double)(n - 1));
}

static void fill_random_block(BYTE* b, size_t n) {
    for (size_t i = 0; i < n; ++i) b[i] = (BYTE)(rand() & 0xFF);
}

int main(void) {
    const char* hexkey = "000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F";
    const char* hexiv = "00000000000000000000000000000000";

    keyInstance enc_key;
    cipherInstance cipher;

    if (cipherInit(&cipher, MODE_ECB, (char*)hexiv) != TRUE) {
        fprintf(stderr, "cipherInit failed\n");
        return 1;
    }

    if (makeKey(&enc_key, DIR_ENCRYPT, 256, (char*)hexkey) != TRUE) {
        fprintf(stderr, "makeKey failed\n");
        return 1;
    }

    BYTE in_fixed[BLOCK_SIZE] = {0};
    BYTE in_random[BLOCK_SIZE];
    BYTE out[BLOCK_SIZE];
    uint64_t t_fixed[SAMPLES];
    uint64_t t_random[SAMPLES];

    srand(0xC0FFEE);

    for (size_t i = 0; i < SAMPLES; ++i) {
        uint64_t start = now_ns();
        blockEncrypt(&cipher, &enc_key, in_fixed, BLOCK_SIZE * 8, out);
        uint64_t end = now_ns();
        t_fixed[i] = end - start;
    }

    for (size_t i = 0; i < SAMPLES; ++i) {
        fill_random_block(in_random, BLOCK_SIZE);
        uint64_t start = now_ns();
        blockEncrypt(&cipher, &enc_key, in_random, BLOCK_SIZE * 8, out);
        uint64_t end = now_ns();
        t_random[i] = end - start;
    }

    double m_fixed = mean_u64(t_fixed, SAMPLES);
    double m_random = mean_u64(t_random, SAMPLES);
    double v_fixed = var_u64(t_fixed, SAMPLES, m_fixed);
    double v_random = var_u64(t_random, SAMPLES, m_random);
    double t_num = m_random - m_fixed;
    double t_den = sqrt((v_fixed / (double)SAMPLES) + (v_random / (double)SAMPLES));
    double t_score = t_num / t_den;

    printf("LOKI97 timing side-channel probe\n");
    printf("Samples per class: %d\n", SAMPLES);
    printf("Mean fixed plaintext time:  %.2f ns\n", m_fixed);
    printf("Mean random plaintext time: %.2f ns\n", m_random);
    printf("Delta (random-fixed):       %.2f ns\n", m_random - m_fixed);
    printf("Welch t-score:              %.2f\n", t_score);

    if (fabs(t_score) > 10.0) {
        printf("RESULT: Timing classes are clearly separable -> side-channel evidence.\n");
        return 2;
    }

    printf("RESULT: No strong class separation in this build/environment.\n");
    return 0;
}
