/***********************************************************************
 * Copyright (c) 2020 Jonas Nick                                       *
 * Distributed under the MIT software license, see the accompanying    *
 * file COPYING or https://www.opensource.org/licenses/mit-license.php.*
 ***********************************************************************/

#ifndef SECP256K1_MODULE_EXTRAKEYS_TESTS_H
#define SECP256K1_MODULE_EXTRAKEYS_TESTS_H

#include "../../../include/secp256k1_extrakeys.h"

static rustsecp256k1zkp_v0_6_0_context* api_test_context(int flags, int *ecount) {
    rustsecp256k1zkp_v0_6_0_context *ctx0 = rustsecp256k1zkp_v0_6_0_context_create(flags);
    rustsecp256k1zkp_v0_6_0_context_set_error_callback(ctx0, counting_illegal_callback_fn, ecount);
    rustsecp256k1zkp_v0_6_0_context_set_illegal_callback(ctx0, counting_illegal_callback_fn, ecount);
    return ctx0;
}

void test_xonly_pubkey(void) {
    rustsecp256k1zkp_v0_6_0_pubkey pk;
    rustsecp256k1zkp_v0_6_0_xonly_pubkey xonly_pk, xonly_pk_tmp;
    rustsecp256k1zkp_v0_6_0_ge pk1;
    rustsecp256k1zkp_v0_6_0_ge pk2;
    rustsecp256k1zkp_v0_6_0_fe y;
    unsigned char sk[32];
    unsigned char xy_sk[32];
    unsigned char buf32[32];
    unsigned char ones32[32];
    unsigned char zeros64[64] = { 0 };
    int pk_parity;
    int i;

    int ecount;
    rustsecp256k1zkp_v0_6_0_context *none = api_test_context(SECP256K1_CONTEXT_NONE, &ecount);
    rustsecp256k1zkp_v0_6_0_context *sign = api_test_context(SECP256K1_CONTEXT_SIGN, &ecount);
    rustsecp256k1zkp_v0_6_0_context *verify = api_test_context(SECP256K1_CONTEXT_VERIFY, &ecount);

    rustsecp256k1zkp_v0_6_0_testrand256(sk);
    memset(ones32, 0xFF, 32);
    rustsecp256k1zkp_v0_6_0_testrand256(xy_sk);
    CHECK(rustsecp256k1zkp_v0_6_0_ec_pubkey_create(sign, &pk, sk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(none, &xonly_pk, &pk_parity, &pk) == 1);

    /* Test xonly_pubkey_from_pubkey */
    ecount = 0;
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(none, &xonly_pk, &pk_parity, &pk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(sign, &xonly_pk, &pk_parity, &pk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(verify, &xonly_pk, &pk_parity, &pk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(none, NULL, &pk_parity, &pk) == 0);
    CHECK(ecount == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(none, &xonly_pk, NULL, &pk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(none, &xonly_pk, &pk_parity, NULL) == 0);
    CHECK(ecount == 2);
    memset(&pk, 0, sizeof(pk));
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(none, &xonly_pk, &pk_parity, &pk) == 0);
    CHECK(ecount == 3);

    /* Choose a secret key such that the resulting pubkey and xonly_pubkey match. */
    memset(sk, 0, sizeof(sk));
    sk[0] = 1;
    CHECK(rustsecp256k1zkp_v0_6_0_ec_pubkey_create(ctx, &pk, sk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(ctx, &xonly_pk, &pk_parity, &pk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&pk, &xonly_pk, sizeof(pk)) == 0);
    CHECK(pk_parity == 0);

    /* Choose a secret key such that pubkey and xonly_pubkey are each others
     * negation. */
    sk[0] = 2;
    CHECK(rustsecp256k1zkp_v0_6_0_ec_pubkey_create(ctx, &pk, sk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(ctx, &xonly_pk, &pk_parity, &pk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&xonly_pk, &pk, sizeof(xonly_pk)) != 0);
    CHECK(pk_parity == 1);
    rustsecp256k1zkp_v0_6_0_pubkey_load(ctx, &pk1, &pk);
    rustsecp256k1zkp_v0_6_0_pubkey_load(ctx, &pk2, (rustsecp256k1zkp_v0_6_0_pubkey *) &xonly_pk);
    CHECK(rustsecp256k1zkp_v0_6_0_fe_equal(&pk1.x, &pk2.x) == 1);
    rustsecp256k1zkp_v0_6_0_fe_negate(&y, &pk2.y, 1);
    CHECK(rustsecp256k1zkp_v0_6_0_fe_equal(&pk1.y, &y) == 1);

    /* Test xonly_pubkey_serialize and xonly_pubkey_parse */
    ecount = 0;
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_serialize(none, NULL, &xonly_pk) == 0);
    CHECK(ecount == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_serialize(none, buf32, NULL) == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(buf32, zeros64, 32) == 0);
    CHECK(ecount == 2);
    {
        /* A pubkey filled with 0s will fail to serialize due to pubkey_load
         * special casing. */
        rustsecp256k1zkp_v0_6_0_xonly_pubkey pk_tmp;
        memset(&pk_tmp, 0, sizeof(pk_tmp));
        CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_serialize(none, buf32, &pk_tmp) == 0);
    }
    /* pubkey_load called illegal callback */
    CHECK(ecount == 3);

    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_serialize(none, buf32, &xonly_pk) == 1);
    ecount = 0;
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_parse(none, NULL, buf32) == 0);
    CHECK(ecount == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_parse(none, &xonly_pk, NULL) == 0);
    CHECK(ecount == 2);

    /* Serialization and parse roundtrip */
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(none, &xonly_pk, NULL, &pk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_serialize(ctx, buf32, &xonly_pk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_parse(ctx, &xonly_pk_tmp, buf32) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&xonly_pk, &xonly_pk_tmp, sizeof(xonly_pk)) == 0);

    /* Test parsing invalid field elements */
    memset(&xonly_pk, 1, sizeof(xonly_pk));
    /* Overflowing field element */
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_parse(none, &xonly_pk, ones32) == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&xonly_pk, zeros64, sizeof(xonly_pk)) == 0);
    memset(&xonly_pk, 1, sizeof(xonly_pk));
    /* There's no point with x-coordinate 0 on secp256k1 */
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_parse(none, &xonly_pk, zeros64) == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&xonly_pk, zeros64, sizeof(xonly_pk)) == 0);
    /* If a random 32-byte string can not be parsed with ec_pubkey_parse
     * (because interpreted as X coordinate it does not correspond to a point on
     * the curve) then xonly_pubkey_parse should fail as well. */
    for (i = 0; i < count; i++) {
        unsigned char rand33[33];
        rustsecp256k1zkp_v0_6_0_testrand256(&rand33[1]);
        rand33[0] = SECP256K1_TAG_PUBKEY_EVEN;
        if (!rustsecp256k1zkp_v0_6_0_ec_pubkey_parse(ctx, &pk, rand33, 33)) {
            memset(&xonly_pk, 1, sizeof(xonly_pk));
            CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_parse(ctx, &xonly_pk, &rand33[1]) == 0);
            CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&xonly_pk, zeros64, sizeof(xonly_pk)) == 0);
        } else {
            CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_parse(ctx, &xonly_pk, &rand33[1]) == 1);
        }
    }
    CHECK(ecount == 2);

    rustsecp256k1zkp_v0_6_0_context_destroy(none);
    rustsecp256k1zkp_v0_6_0_context_destroy(sign);
    rustsecp256k1zkp_v0_6_0_context_destroy(verify);
}

void test_xonly_pubkey_comparison(void) {
    unsigned char pk1_ser[32] = {
        0x58, 0x84, 0xb3, 0xa2, 0x4b, 0x97, 0x37, 0x88, 0x92, 0x38, 0xa6, 0x26, 0x62, 0x52, 0x35, 0x11,
        0xd0, 0x9a, 0xa1, 0x1b, 0x80, 0x0b, 0x5e, 0x93, 0x80, 0x26, 0x11, 0xef, 0x67, 0x4b, 0xd9, 0x23
    };
    const unsigned char pk2_ser[32] = {
        0xde, 0x36, 0x0e, 0x87, 0x59, 0x8f, 0x3c, 0x01, 0x36, 0x2a, 0x2a, 0xb8, 0xc6, 0xf4, 0x5e, 0x4d,
        0xb2, 0xc2, 0xd5, 0x03, 0xa7, 0xf9, 0xf1, 0x4f, 0xa8, 0xfa, 0x95, 0xa8, 0xe9, 0x69, 0x76, 0x1c
    };
    rustsecp256k1zkp_v0_6_0_xonly_pubkey pk1;
    rustsecp256k1zkp_v0_6_0_xonly_pubkey pk2;
    int ecount = 0;
    rustsecp256k1zkp_v0_6_0_context *none = api_test_context(SECP256K1_CONTEXT_NONE, &ecount);

    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_parse(none, &pk1, pk1_ser) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_parse(none, &pk2, pk2_ser) == 1);

    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_cmp(none, NULL, &pk2) < 0);
    CHECK(ecount == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_cmp(none, &pk1, NULL) > 0);
    CHECK(ecount == 2);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_cmp(none, &pk1, &pk2) < 0);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_cmp(none, &pk2, &pk1) > 0);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_cmp(none, &pk1, &pk1) == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_cmp(none, &pk2, &pk2) == 0);
    CHECK(ecount == 2);
    memset(&pk1, 0, sizeof(pk1)); /* illegal pubkey */
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_cmp(none, &pk1, &pk2) < 0);
    CHECK(ecount == 3);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_cmp(none, &pk1, &pk1) == 0);
    CHECK(ecount == 5);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_cmp(none, &pk2, &pk1) > 0);
    CHECK(ecount == 6);

    rustsecp256k1zkp_v0_6_0_context_destroy(none);
}

void test_xonly_pubkey_tweak(void) {
    unsigned char zeros64[64] = { 0 };
    unsigned char overflows[32];
    unsigned char sk[32];
    rustsecp256k1zkp_v0_6_0_pubkey internal_pk;
    rustsecp256k1zkp_v0_6_0_xonly_pubkey internal_xonly_pk;
    rustsecp256k1zkp_v0_6_0_pubkey output_pk;
    int pk_parity;
    unsigned char tweak[32];
    int i;

    int ecount;
    rustsecp256k1zkp_v0_6_0_context *none = api_test_context(SECP256K1_CONTEXT_NONE, &ecount);
    rustsecp256k1zkp_v0_6_0_context *sign = api_test_context(SECP256K1_CONTEXT_SIGN, &ecount);
    rustsecp256k1zkp_v0_6_0_context *verify = api_test_context(SECP256K1_CONTEXT_VERIFY, &ecount);

    memset(overflows, 0xff, sizeof(overflows));
    rustsecp256k1zkp_v0_6_0_testrand256(tweak);
    rustsecp256k1zkp_v0_6_0_testrand256(sk);
    CHECK(rustsecp256k1zkp_v0_6_0_ec_pubkey_create(ctx, &internal_pk, sk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(none, &internal_xonly_pk, &pk_parity, &internal_pk) == 1);

    ecount = 0;
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add(none, &output_pk, &internal_xonly_pk, tweak) == 1);
    CHECK(ecount == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add(sign, &output_pk, &internal_xonly_pk, tweak) == 1);
    CHECK(ecount == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add(verify, &output_pk, &internal_xonly_pk, tweak) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add(verify, NULL, &internal_xonly_pk, tweak) == 0);
    CHECK(ecount == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add(verify, &output_pk, NULL, tweak) == 0);
    CHECK(ecount == 2);
    /* NULL internal_xonly_pk zeroes the output_pk */
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&output_pk, zeros64, sizeof(output_pk)) == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add(verify, &output_pk, &internal_xonly_pk, NULL) == 0);
    CHECK(ecount == 3);
    /* NULL tweak zeroes the output_pk */
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&output_pk, zeros64, sizeof(output_pk)) == 0);

    /* Invalid tweak zeroes the output_pk */
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add(verify, &output_pk, &internal_xonly_pk, overflows) == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&output_pk, zeros64, sizeof(output_pk))  == 0);

    /* A zero tweak is fine */
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add(verify, &output_pk, &internal_xonly_pk, zeros64) == 1);

    /* Fails if the resulting key was infinity */
    for (i = 0; i < count; i++) {
        rustsecp256k1zkp_v0_6_0_scalar scalar_tweak;
        /* Because sk may be negated before adding, we need to try with tweak =
         * sk as well as tweak = -sk. */
        rustsecp256k1zkp_v0_6_0_scalar_set_b32(&scalar_tweak, sk, NULL);
        rustsecp256k1zkp_v0_6_0_scalar_negate(&scalar_tweak, &scalar_tweak);
        rustsecp256k1zkp_v0_6_0_scalar_get_b32(tweak, &scalar_tweak);
        CHECK((rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add(verify, &output_pk, &internal_xonly_pk, sk) == 0)
              || (rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add(verify, &output_pk, &internal_xonly_pk, tweak) == 0));
        CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&output_pk, zeros64, sizeof(output_pk)) == 0);
    }

    /* Invalid pk with a valid tweak */
    memset(&internal_xonly_pk, 0, sizeof(internal_xonly_pk));
    rustsecp256k1zkp_v0_6_0_testrand256(tweak);
    ecount = 0;
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add(verify, &output_pk, &internal_xonly_pk, tweak) == 0);
    CHECK(ecount == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&output_pk, zeros64, sizeof(output_pk))  == 0);

    rustsecp256k1zkp_v0_6_0_context_destroy(none);
    rustsecp256k1zkp_v0_6_0_context_destroy(sign);
    rustsecp256k1zkp_v0_6_0_context_destroy(verify);
}

void test_xonly_pubkey_tweak_check(void) {
    unsigned char zeros64[64] = { 0 };
    unsigned char overflows[32];
    unsigned char sk[32];
    rustsecp256k1zkp_v0_6_0_pubkey internal_pk;
    rustsecp256k1zkp_v0_6_0_xonly_pubkey internal_xonly_pk;
    rustsecp256k1zkp_v0_6_0_pubkey output_pk;
    rustsecp256k1zkp_v0_6_0_xonly_pubkey output_xonly_pk;
    unsigned char output_pk32[32];
    unsigned char buf32[32];
    int pk_parity;
    unsigned char tweak[32];

    int ecount;
    rustsecp256k1zkp_v0_6_0_context *none = api_test_context(SECP256K1_CONTEXT_NONE, &ecount);
    rustsecp256k1zkp_v0_6_0_context *sign = api_test_context(SECP256K1_CONTEXT_SIGN, &ecount);
    rustsecp256k1zkp_v0_6_0_context *verify = api_test_context(SECP256K1_CONTEXT_VERIFY, &ecount);

    memset(overflows, 0xff, sizeof(overflows));
    rustsecp256k1zkp_v0_6_0_testrand256(tweak);
    rustsecp256k1zkp_v0_6_0_testrand256(sk);
    CHECK(rustsecp256k1zkp_v0_6_0_ec_pubkey_create(ctx, &internal_pk, sk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(none, &internal_xonly_pk, &pk_parity, &internal_pk) == 1);

    ecount = 0;
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add(verify, &output_pk, &internal_xonly_pk, tweak) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(verify, &output_xonly_pk, &pk_parity, &output_pk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_serialize(ctx, buf32, &output_xonly_pk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add_check(none, buf32, pk_parity, &internal_xonly_pk, tweak) == 1);
    CHECK(ecount == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add_check(sign, buf32, pk_parity, &internal_xonly_pk, tweak) == 1);
    CHECK(ecount == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add_check(verify, buf32, pk_parity, &internal_xonly_pk, tweak) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add_check(verify, NULL, pk_parity, &internal_xonly_pk, tweak) == 0);
    CHECK(ecount == 1);
    /* invalid pk_parity value */
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add_check(verify, buf32, 2, &internal_xonly_pk, tweak) == 0);
    CHECK(ecount == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add_check(verify, buf32, pk_parity, NULL, tweak) == 0);
    CHECK(ecount == 2);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add_check(verify, buf32, pk_parity, &internal_xonly_pk, NULL) == 0);
    CHECK(ecount == 3);

    memset(tweak, 1, sizeof(tweak));
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(ctx, &internal_xonly_pk, NULL, &internal_pk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add(ctx, &output_pk, &internal_xonly_pk, tweak) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(ctx, &output_xonly_pk, &pk_parity, &output_pk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_serialize(ctx, output_pk32, &output_xonly_pk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add_check(ctx, output_pk32, pk_parity, &internal_xonly_pk, tweak) == 1);

    /* Wrong pk_parity */
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add_check(ctx, output_pk32, !pk_parity, &internal_xonly_pk, tweak) == 0);
    /* Wrong public key */
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_serialize(ctx, buf32, &internal_xonly_pk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add_check(ctx, buf32, pk_parity, &internal_xonly_pk, tweak) == 0);

    /* Overflowing tweak not allowed */
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add_check(ctx, output_pk32, pk_parity, &internal_xonly_pk, overflows) == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add(ctx, &output_pk, &internal_xonly_pk, overflows) == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&output_pk, zeros64, sizeof(output_pk)) == 0);
    CHECK(ecount == 3);

    rustsecp256k1zkp_v0_6_0_context_destroy(none);
    rustsecp256k1zkp_v0_6_0_context_destroy(sign);
    rustsecp256k1zkp_v0_6_0_context_destroy(verify);
}

/* Starts with an initial pubkey and recursively creates N_PUBKEYS - 1
 * additional pubkeys by calling tweak_add. Then verifies every tweak starting
 * from the last pubkey. */
#define N_PUBKEYS 32
void test_xonly_pubkey_tweak_recursive(void) {
    unsigned char sk[32];
    rustsecp256k1zkp_v0_6_0_pubkey pk[N_PUBKEYS];
    unsigned char pk_serialized[32];
    unsigned char tweak[N_PUBKEYS - 1][32];
    int i;

    rustsecp256k1zkp_v0_6_0_testrand256(sk);
    CHECK(rustsecp256k1zkp_v0_6_0_ec_pubkey_create(ctx, &pk[0], sk) == 1);
    /* Add tweaks */
    for (i = 0; i < N_PUBKEYS - 1; i++) {
        rustsecp256k1zkp_v0_6_0_xonly_pubkey xonly_pk;
        memset(tweak[i], i + 1, sizeof(tweak[i]));
        CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(ctx, &xonly_pk, NULL, &pk[i]) == 1);
        CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add(ctx, &pk[i + 1], &xonly_pk, tweak[i]) == 1);
    }

    /* Verify tweaks */
    for (i = N_PUBKEYS - 1; i > 0; i--) {
        rustsecp256k1zkp_v0_6_0_xonly_pubkey xonly_pk;
        int pk_parity;
        CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(ctx, &xonly_pk, &pk_parity, &pk[i]) == 1);
        CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_serialize(ctx, pk_serialized, &xonly_pk) == 1);
        CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(ctx, &xonly_pk, NULL, &pk[i - 1]) == 1);
        CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add_check(ctx, pk_serialized, pk_parity, &xonly_pk, tweak[i - 1]) == 1);
    }
}
#undef N_PUBKEYS

void test_keypair(void) {
    unsigned char sk[32];
    unsigned char sk_tmp[32];
    unsigned char zeros96[96] = { 0 };
    unsigned char overflows[32];
    rustsecp256k1zkp_v0_6_0_keypair keypair;
    rustsecp256k1zkp_v0_6_0_pubkey pk, pk_tmp;
    rustsecp256k1zkp_v0_6_0_xonly_pubkey xonly_pk, xonly_pk_tmp;
    int pk_parity, pk_parity_tmp;
    int ecount;
    rustsecp256k1zkp_v0_6_0_context *none = api_test_context(SECP256K1_CONTEXT_NONE, &ecount);
    rustsecp256k1zkp_v0_6_0_context *sign = api_test_context(SECP256K1_CONTEXT_SIGN, &ecount);
    rustsecp256k1zkp_v0_6_0_context *verify = api_test_context(SECP256K1_CONTEXT_VERIFY, &ecount);
    rustsecp256k1zkp_v0_6_0_context *sttc = rustsecp256k1zkp_v0_6_0_context_clone(rustsecp256k1zkp_v0_6_0_context_no_precomp);
    rustsecp256k1zkp_v0_6_0_context_set_error_callback(sttc, counting_illegal_callback_fn, &ecount);
    rustsecp256k1zkp_v0_6_0_context_set_illegal_callback(sttc, counting_illegal_callback_fn, &ecount);

    CHECK(sizeof(zeros96) == sizeof(keypair));
    memset(overflows, 0xFF, sizeof(overflows));

    /* Test keypair_create */
    ecount = 0;
    rustsecp256k1zkp_v0_6_0_testrand256(sk);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(none, &keypair, sk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(zeros96, &keypair, sizeof(keypair)) != 0);
    CHECK(ecount == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(verify, &keypair, sk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(zeros96, &keypair, sizeof(keypair)) != 0);
    CHECK(ecount == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(sign, NULL, sk) == 0);
    CHECK(ecount == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(sign, &keypair, NULL) == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(zeros96, &keypair, sizeof(keypair)) == 0);
    CHECK(ecount == 2);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(sign, &keypair, sk) == 1);
    CHECK(ecount == 2);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(sttc, &keypair, sk) == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(zeros96, &keypair, sizeof(keypair)) == 0);
    CHECK(ecount == 3);

    /* Invalid secret key */
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(sign, &keypair, zeros96) == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(zeros96, &keypair, sizeof(keypair)) == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(sign, &keypair, overflows) == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(zeros96, &keypair, sizeof(keypair)) == 0);

    /* Test keypair_pub */
    ecount = 0;
    rustsecp256k1zkp_v0_6_0_testrand256(sk);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(ctx, &keypair, sk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_pub(none, &pk, &keypair) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_pub(none, NULL, &keypair) == 0);
    CHECK(ecount == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_pub(none, &pk, NULL) == 0);
    CHECK(ecount == 2);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(zeros96, &pk, sizeof(pk)) == 0);

    /* Using an invalid keypair is fine for keypair_pub */
    memset(&keypair, 0, sizeof(keypair));
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_pub(none, &pk, &keypair) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(zeros96, &pk, sizeof(pk)) == 0);

    /* keypair holds the same pubkey as pubkey_create */
    CHECK(rustsecp256k1zkp_v0_6_0_ec_pubkey_create(sign, &pk, sk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(sign, &keypair, sk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_pub(none, &pk_tmp, &keypair) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&pk, &pk_tmp, sizeof(pk)) == 0);

    /** Test keypair_xonly_pub **/
    ecount = 0;
    rustsecp256k1zkp_v0_6_0_testrand256(sk);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(ctx, &keypair, sk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_pub(none, &xonly_pk, &pk_parity, &keypair) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_pub(none, NULL, &pk_parity, &keypair) == 0);
    CHECK(ecount == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_pub(none, &xonly_pk, NULL, &keypair) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_pub(none, &xonly_pk, &pk_parity, NULL) == 0);
    CHECK(ecount == 2);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(zeros96, &xonly_pk, sizeof(xonly_pk)) == 0);
    /* Using an invalid keypair will set the xonly_pk to 0 (first reset
     * xonly_pk). */
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_pub(none, &xonly_pk, &pk_parity, &keypair) == 1);
    memset(&keypair, 0, sizeof(keypair));
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_pub(none, &xonly_pk, &pk_parity, &keypair) == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(zeros96, &xonly_pk, sizeof(xonly_pk)) == 0);
    CHECK(ecount == 3);

    /** keypair holds the same xonly pubkey as pubkey_create **/
    CHECK(rustsecp256k1zkp_v0_6_0_ec_pubkey_create(sign, &pk, sk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_from_pubkey(none, &xonly_pk, &pk_parity, &pk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(sign, &keypair, sk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_pub(none, &xonly_pk_tmp, &pk_parity_tmp, &keypair) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&xonly_pk, &xonly_pk_tmp, sizeof(pk)) == 0);
    CHECK(pk_parity == pk_parity_tmp);

    /* Test keypair_seckey */
    ecount = 0;
    rustsecp256k1zkp_v0_6_0_testrand256(sk);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(ctx, &keypair, sk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_sec(none, sk_tmp, &keypair) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_sec(none, NULL, &keypair) == 0);
    CHECK(ecount == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_sec(none, sk_tmp, NULL) == 0);
    CHECK(ecount == 2);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(zeros96, sk_tmp, sizeof(sk_tmp)) == 0);

    /* keypair returns the same seckey it got */
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(sign, &keypair, sk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_sec(none, sk_tmp, &keypair) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(sk, sk_tmp, sizeof(sk_tmp)) == 0);


    /* Using an invalid keypair is fine for keypair_seckey */
    memset(&keypair, 0, sizeof(keypair));
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_sec(none, sk_tmp, &keypair) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(zeros96, sk_tmp, sizeof(sk_tmp)) == 0);

    rustsecp256k1zkp_v0_6_0_context_destroy(none);
    rustsecp256k1zkp_v0_6_0_context_destroy(sign);
    rustsecp256k1zkp_v0_6_0_context_destroy(verify);
    rustsecp256k1zkp_v0_6_0_context_destroy(sttc);
}

void test_keypair_add(void) {
    unsigned char sk[32];
    rustsecp256k1zkp_v0_6_0_keypair keypair;
    unsigned char overflows[32];
    unsigned char zeros96[96] = { 0 };
    unsigned char tweak[32];
    int i;
    int ecount = 0;
    rustsecp256k1zkp_v0_6_0_context *none = api_test_context(SECP256K1_CONTEXT_NONE, &ecount);
    rustsecp256k1zkp_v0_6_0_context *sign = api_test_context(SECP256K1_CONTEXT_SIGN, &ecount);
    rustsecp256k1zkp_v0_6_0_context *verify = api_test_context(SECP256K1_CONTEXT_VERIFY, &ecount);

    CHECK(sizeof(zeros96) == sizeof(keypair));
    rustsecp256k1zkp_v0_6_0_testrand256(sk);
    rustsecp256k1zkp_v0_6_0_testrand256(tweak);
    memset(overflows, 0xFF, 32);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(ctx, &keypair, sk) == 1);

    CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_tweak_add(none, &keypair, tweak) == 1);
    CHECK(ecount == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_tweak_add(sign, &keypair, tweak) == 1);
    CHECK(ecount == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_tweak_add(verify, &keypair, tweak) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_tweak_add(verify, NULL, tweak) == 0);
    CHECK(ecount == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_tweak_add(verify, &keypair, NULL) == 0);
    CHECK(ecount == 2);
    /* This does not set the keypair to zeroes */
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&keypair, zeros96, sizeof(keypair)) != 0);

    /* Invalid tweak zeroes the keypair */
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(ctx, &keypair, sk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_tweak_add(ctx, &keypair, overflows) == 0);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&keypair, zeros96, sizeof(keypair))  == 0);

    /* A zero tweak is fine */
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(ctx, &keypair, sk) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_tweak_add(ctx, &keypair, zeros96) == 1);

    /* Fails if the resulting keypair was (sk=0, pk=infinity) */
    for (i = 0; i < count; i++) {
        rustsecp256k1zkp_v0_6_0_scalar scalar_tweak;
        rustsecp256k1zkp_v0_6_0_keypair keypair_tmp;
        rustsecp256k1zkp_v0_6_0_testrand256(sk);
        CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(ctx, &keypair, sk) == 1);
        memcpy(&keypair_tmp, &keypair, sizeof(keypair));
        /* Because sk may be negated before adding, we need to try with tweak =
         * sk as well as tweak = -sk. */
        rustsecp256k1zkp_v0_6_0_scalar_set_b32(&scalar_tweak, sk, NULL);
        rustsecp256k1zkp_v0_6_0_scalar_negate(&scalar_tweak, &scalar_tweak);
        rustsecp256k1zkp_v0_6_0_scalar_get_b32(tweak, &scalar_tweak);
        CHECK((rustsecp256k1zkp_v0_6_0_keypair_xonly_tweak_add(ctx, &keypair, sk) == 0)
              || (rustsecp256k1zkp_v0_6_0_keypair_xonly_tweak_add(ctx, &keypair_tmp, tweak) == 0));
        CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&keypair, zeros96, sizeof(keypair)) == 0
              || rustsecp256k1zkp_v0_6_0_memcmp_var(&keypair_tmp, zeros96, sizeof(keypair_tmp)) == 0);
    }

    /* Invalid keypair with a valid tweak */
    memset(&keypair, 0, sizeof(keypair));
    rustsecp256k1zkp_v0_6_0_testrand256(tweak);
    ecount = 0;
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_tweak_add(verify, &keypair, tweak) == 0);
    CHECK(ecount == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&keypair, zeros96, sizeof(keypair))  == 0);
    /* Only seckey part of keypair invalid */
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(ctx, &keypair, sk) == 1);
    memset(&keypair, 0, 32);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_tweak_add(verify, &keypair, tweak) == 0);
    CHECK(ecount == 2);
    /* Only pubkey part of keypair invalid */
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(ctx, &keypair, sk) == 1);
    memset(&keypair.data[32], 0, 64);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_tweak_add(verify, &keypair, tweak) == 0);
    CHECK(ecount == 3);

    /* Check that the keypair_tweak_add implementation is correct */
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(ctx, &keypair, sk) == 1);
    for (i = 0; i < count; i++) {
        rustsecp256k1zkp_v0_6_0_xonly_pubkey internal_pk;
        rustsecp256k1zkp_v0_6_0_xonly_pubkey output_pk;
        rustsecp256k1zkp_v0_6_0_pubkey output_pk_xy;
        rustsecp256k1zkp_v0_6_0_pubkey output_pk_expected;
        unsigned char pk32[32];
        unsigned char sk32[32];
        int pk_parity;

        rustsecp256k1zkp_v0_6_0_testrand256(tweak);
        CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_pub(ctx, &internal_pk, NULL, &keypair) == 1);
        CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_tweak_add(ctx, &keypair, tweak) == 1);
        CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_pub(ctx, &output_pk, &pk_parity, &keypair) == 1);

        /* Check that it passes xonly_pubkey_tweak_add_check */
        CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_serialize(ctx, pk32, &output_pk) == 1);
        CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add_check(ctx, pk32, pk_parity, &internal_pk, tweak) == 1);

        /* Check that the resulting pubkey matches xonly_pubkey_tweak_add */
        CHECK(rustsecp256k1zkp_v0_6_0_keypair_pub(ctx, &output_pk_xy, &keypair) == 1);
        CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_tweak_add(ctx, &output_pk_expected, &internal_pk, tweak) == 1);
        CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&output_pk_xy, &output_pk_expected, sizeof(output_pk_xy)) == 0);

        /* Check that the secret key in the keypair is tweaked correctly */
        CHECK(rustsecp256k1zkp_v0_6_0_keypair_sec(none, sk32, &keypair) == 1);
        CHECK(rustsecp256k1zkp_v0_6_0_ec_pubkey_create(ctx, &output_pk_expected, sk32) == 1);
        CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(&output_pk_xy, &output_pk_expected, sizeof(output_pk_xy)) == 0);
    }
    rustsecp256k1zkp_v0_6_0_context_destroy(none);
    rustsecp256k1zkp_v0_6_0_context_destroy(sign);
    rustsecp256k1zkp_v0_6_0_context_destroy(verify);
}

static void test_hsort_is_sorted(int *ints, size_t n) {
    size_t i;
    for (i = 1; i < n; i++) {
        CHECK(ints[i-1] <= ints[i]);
    }
}

static int test_hsort_cmp(const void *i1, const void *i2, void *counter) {
  *(size_t*)counter += 1;
  return *(int*)i1 - *(int*)i2;
}

#define NUM 64
void test_hsort(void) {
    int ints[NUM] = { 0 };
    size_t counter = 0;
    int i, j;

    rustsecp256k1zkp_v0_6_0_hsort(ints, 0, sizeof(ints[0]), test_hsort_cmp, &counter);
    CHECK(counter == 0);
    rustsecp256k1zkp_v0_6_0_hsort(ints, 1, sizeof(ints[0]), test_hsort_cmp, &counter);
    CHECK(counter == 0);
    rustsecp256k1zkp_v0_6_0_hsort(ints, NUM, sizeof(ints[0]), test_hsort_cmp, &counter);
    CHECK(counter > 0);
    test_hsort_is_sorted(ints, NUM);

    /* Test hsort with length n array and random elements in
     * [-interval/2, interval/2] */
    for (i = 0; i < count; i++) {
        int n = rustsecp256k1zkp_v0_6_0_testrand_int(NUM);
        int interval = rustsecp256k1zkp_v0_6_0_testrand_int(64);
        for (j = 0; j < n; j++) {
            ints[j] = rustsecp256k1zkp_v0_6_0_testrand_int(interval) - interval/2;
        }
        rustsecp256k1zkp_v0_6_0_hsort(ints, n, sizeof(ints[0]), test_hsort_cmp, &counter);
        test_hsort_is_sorted(ints, n);
    }
}
#undef NUM

void test_xonly_sort_helper(rustsecp256k1zkp_v0_6_0_xonly_pubkey *pk, size_t *pk_order, size_t n_pk) {
    size_t i;
    const rustsecp256k1zkp_v0_6_0_xonly_pubkey *pk_test[5];

    for (i = 0; i < n_pk; i++) {
        pk_test[i] = &pk[pk_order[i]];
    }
    rustsecp256k1zkp_v0_6_0_xonly_sort(ctx, pk_test, n_pk);
    for (i = 0; i < n_pk; i++) {
        CHECK(rustsecp256k1zkp_v0_6_0_memcmp_var(pk_test[i], &pk[i], sizeof(*pk_test[i])) == 0);
    }
}

void permute(size_t *arr, size_t n) {
    size_t i;
    for (i = n - 1; i >= 1; i--) {
        size_t tmp, j;
        j = rustsecp256k1zkp_v0_6_0_testrand_int(i + 1);
        tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

void rand_xonly_pk(rustsecp256k1zkp_v0_6_0_xonly_pubkey *pk) {
    unsigned char seckey[32];
    rustsecp256k1zkp_v0_6_0_keypair keypair;
    rustsecp256k1zkp_v0_6_0_testrand256(seckey);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_create(ctx, &keypair, seckey) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_keypair_xonly_pub(ctx, pk, NULL, &keypair) == 1);
}

void test_xonly_sort_api(void) {
    int ecount = 0;
    rustsecp256k1zkp_v0_6_0_xonly_pubkey pks[2];
    const rustsecp256k1zkp_v0_6_0_xonly_pubkey *pks_ptr[2];
    rustsecp256k1zkp_v0_6_0_context *none = api_test_context(SECP256K1_CONTEXT_NONE, &ecount);

    pks_ptr[0] = &pks[0];
    pks_ptr[1] = &pks[1];

    rand_xonly_pk(&pks[0]);
    rand_xonly_pk(&pks[1]);

    CHECK(rustsecp256k1zkp_v0_6_0_xonly_sort(none, pks_ptr, 2) == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_sort(none, NULL, 2) == 0);
    CHECK(ecount == 1);
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_sort(none, pks_ptr, 0) == 1);
    /* Test illegal public keys */
    memset(&pks[0], 0, sizeof(pks[0]));
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_sort(none, pks_ptr, 2) == 1);
    CHECK(ecount == 2);
    memset(&pks[1], 0, sizeof(pks[1]));
    CHECK(rustsecp256k1zkp_v0_6_0_xonly_sort(none, pks_ptr, 2) == 1);
    CHECK(ecount > 2);

    rustsecp256k1zkp_v0_6_0_context_destroy(none);
}

void test_xonly_sort(void) {
    rustsecp256k1zkp_v0_6_0_xonly_pubkey pk[5];
    unsigned char pk_ser[5][32];
    int i;
    size_t pk_order[5] = { 0, 1, 2, 3, 4 };

    for (i = 0; i < 5; i++) {
        memset(pk_ser[i], 0, sizeof(pk_ser[i]));
    }
    pk_ser[0][0] = 5;
    pk_ser[1][0] = 8;
    pk_ser[2][0] = 0x0a;
    pk_ser[3][0] = 0x0b;
    pk_ser[4][0] = 0x0c;
    for (i = 0; i < 5; i++) {
        CHECK(rustsecp256k1zkp_v0_6_0_xonly_pubkey_parse(ctx, &pk[i], pk_ser[i]));
    }

    permute(pk_order, 1);
    test_xonly_sort_helper(pk, pk_order, 1);
    permute(pk_order, 2);
    test_xonly_sort_helper(pk, pk_order, 2);
    permute(pk_order, 3);
    test_xonly_sort_helper(pk, pk_order, 3);
    for (i = 0; i < count; i++) {
        permute(pk_order, 4);
        test_xonly_sort_helper(pk, pk_order, 4);
    }
    for (i = 0; i < count; i++) {
        permute(pk_order, 5);
        test_xonly_sort_helper(pk, pk_order, 5);
    }
    /* Check that sorting also works for random pubkeys */
    for (i = 0; i < count; i++) {
        int j;
        const rustsecp256k1zkp_v0_6_0_xonly_pubkey *pk_ptr[5];
        for (j = 0; j < 5; j++) {
            rand_xonly_pk(&pk[j]);
            pk_ptr[j] = &pk[j];
        }
        rustsecp256k1zkp_v0_6_0_xonly_sort(ctx, pk_ptr, 5);
        for (j = 1; j < 5; j++) {
            CHECK(rustsecp256k1zkp_v0_6_0_xonly_sort_cmp(&pk_ptr[j - 1], &pk_ptr[j], ctx) <= 0);
        }
    }
}

void run_extrakeys_tests(void) {
    /* xonly key test cases */
    test_xonly_pubkey();
    test_xonly_pubkey_tweak();
    test_xonly_pubkey_tweak_check();
    test_xonly_pubkey_tweak_recursive();
    test_xonly_pubkey_comparison();

    /* keypair tests */
    test_keypair();
    test_keypair_add();

    test_hsort();
    test_xonly_sort_api();
    test_xonly_sort();
}

#endif
