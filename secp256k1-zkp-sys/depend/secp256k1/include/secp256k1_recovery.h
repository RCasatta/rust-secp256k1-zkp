#ifndef SECP256K1_RECOVERY_H
#define SECP256K1_RECOVERY_H

#include "secp256k1.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque data structured that holds a parsed ECDSA signature,
 *  supporting pubkey recovery.
 *
 *  The exact representation of data inside is implementation defined and not
 *  guaranteed to be portable between different platforms or versions. It is
 *  however guaranteed to be 65 bytes in size, and can be safely copied/moved.
 *  If you need to convert to a format suitable for storage or transmission, use
 *  the rustsecp256k1zkp_v0_8_0_ecdsa_signature_serialize_* and
 *  rustsecp256k1zkp_v0_8_0_ecdsa_signature_parse_* functions.
 *
 *  Furthermore, it is guaranteed that identical signatures (including their
 *  recoverability) will have identical representation, so they can be
 *  memcmp'ed.
 */
typedef struct {
    unsigned char data[65];
} rustsecp256k1zkp_v0_8_0_ecdsa_recoverable_signature;

/** Parse a compact ECDSA signature (64 bytes + recovery id).
 *
 *  Returns: 1 when the signature could be parsed, 0 otherwise
 *  Args: ctx:     a secp256k1 context object
 *  Out:  sig:     a pointer to a signature object
 *  In:   input64: a pointer to a 64-byte compact signature
 *        recid:   the recovery id (0, 1, 2 or 3)
 */
SECP256K1_API int rustsecp256k1zkp_v0_8_0_ecdsa_recoverable_signature_parse_compact(
    const rustsecp256k1zkp_v0_8_0_context* ctx,
    rustsecp256k1zkp_v0_8_0_ecdsa_recoverable_signature* sig,
    const unsigned char *input64,
    int recid
) SECP256K1_ARG_NONNULL(1) SECP256K1_ARG_NONNULL(2) SECP256K1_ARG_NONNULL(3);

/** Convert a recoverable signature into a normal signature.
 *
 *  Returns: 1
 *  Args: ctx:    a secp256k1 context object.
 *  Out:  sig:    a pointer to a normal signature.
 *  In:   sigin:  a pointer to a recoverable signature.
 */
SECP256K1_API int rustsecp256k1zkp_v0_8_0_ecdsa_recoverable_signature_convert(
    const rustsecp256k1zkp_v0_8_0_context* ctx,
    rustsecp256k1zkp_v0_8_0_ecdsa_signature* sig,
    const rustsecp256k1zkp_v0_8_0_ecdsa_recoverable_signature* sigin
) SECP256K1_ARG_NONNULL(1) SECP256K1_ARG_NONNULL(2) SECP256K1_ARG_NONNULL(3);

/** Serialize an ECDSA signature in compact format (64 bytes + recovery id).
 *
 *  Returns: 1
 *  Args: ctx:      a secp256k1 context object.
 *  Out:  output64: a pointer to a 64-byte array of the compact signature.
 *        recid:    a pointer to an integer to hold the recovery id.
 *  In:   sig:      a pointer to an initialized signature object.
 */
SECP256K1_API int rustsecp256k1zkp_v0_8_0_ecdsa_recoverable_signature_serialize_compact(
    const rustsecp256k1zkp_v0_8_0_context* ctx,
    unsigned char *output64,
    int *recid,
    const rustsecp256k1zkp_v0_8_0_ecdsa_recoverable_signature* sig
) SECP256K1_ARG_NONNULL(1) SECP256K1_ARG_NONNULL(2) SECP256K1_ARG_NONNULL(3) SECP256K1_ARG_NONNULL(4);

/** Create a recoverable ECDSA signature.
 *
 *  Returns: 1: signature created
 *           0: the nonce generation function failed, or the secret key was invalid.
 *  Args:    ctx:       pointer to a context object, initialized for signing.
 *  Out:     sig:       pointer to an array where the signature will be placed.
 *  In:      msghash32: the 32-byte message hash being signed.
 *           seckey:    pointer to a 32-byte secret key.
 *           noncefp:   pointer to a nonce generation function. If NULL,
 *                      rustsecp256k1zkp_v0_8_0_nonce_function_default is used.
 *           ndata:     pointer to arbitrary data used by the nonce generation function
 *                      (can be NULL for rustsecp256k1zkp_v0_8_0_nonce_function_default).
 */
SECP256K1_API int rustsecp256k1zkp_v0_8_0_ecdsa_sign_recoverable(
    const rustsecp256k1zkp_v0_8_0_context* ctx,
    rustsecp256k1zkp_v0_8_0_ecdsa_recoverable_signature *sig,
    const unsigned char *msghash32,
    const unsigned char *seckey,
    rustsecp256k1zkp_v0_8_0_nonce_function noncefp,
    const void *ndata
) SECP256K1_ARG_NONNULL(1) SECP256K1_ARG_NONNULL(2) SECP256K1_ARG_NONNULL(3) SECP256K1_ARG_NONNULL(4);

/** Recover an ECDSA public key from a signature.
 *
 *  Returns: 1: public key successfully recovered (which guarantees a correct signature).
 *           0: otherwise.
 *  Args:    ctx:       pointer to a context object, initialized for verification.
 *  Out:     pubkey:    pointer to the recovered public key.
 *  In:      sig:       pointer to initialized signature that supports pubkey recovery.
 *           msghash32: the 32-byte message hash assumed to be signed.
 */
SECP256K1_API SECP256K1_WARN_UNUSED_RESULT int rustsecp256k1zkp_v0_8_0_ecdsa_recover(
    const rustsecp256k1zkp_v0_8_0_context* ctx,
    rustsecp256k1zkp_v0_8_0_pubkey *pubkey,
    const rustsecp256k1zkp_v0_8_0_ecdsa_recoverable_signature *sig,
    const unsigned char *msghash32
) SECP256K1_ARG_NONNULL(1) SECP256K1_ARG_NONNULL(2) SECP256K1_ARG_NONNULL(3) SECP256K1_ARG_NONNULL(4);

#ifdef __cplusplus
}
#endif

#endif /* SECP256K1_RECOVERY_H */
