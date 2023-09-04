#include "sign_check.h"
#include "cmox_crypto.h"
#include "main.h"

cmox_ecc_handle_t Ecc_Ctx;
/* ECC working buffer */
uint8_t Working_Buffer[2000];

/* Random data buffer */
uint8_t Computed_Hash[CMOX_SHA256_SIZE] = {0};

const uint8_t Public_Key[] = {
    0x8d, 0x56, 0x39, 0xbb, 0xef, 0x40, 0x55, 0x42,
    0x27, 0xa4, 0xc3, 0x14, 0x2a, 0x07, 0xec, 0xd9,
    0x60, 0xd6, 0xdc, 0xe3, 0x84, 0x26, 0x5f, 0xf0,
    0xe6, 0x23, 0x06, 0x7a, 0xba, 0x0d, 0x54, 0x4f,
    0x8f, 0x8b, 0xe2, 0xa2, 0x22, 0xd9, 0x5f, 0xbb,
    0xbe, 0x57, 0x88, 0x08, 0x9a, 0xbf, 0xc2, 0x4c,
    0xa1, 0x2f, 0x2a, 0x29, 0x71, 0x01, 0xfe, 0x8f,
    0x43, 0x16, 0x62, 0xd3, 0x61, 0xa4, 0x5f, 0x92};

void SignCheck_Init(void)
{
    if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
    {
        __BKPT(1);
    }
}

CL_Result_t SingCheck(const uint8_t *data, uint32_t dataSize, const uint8_t *sign, uint32_t signSize)
{
    cmox_hash_retval_t hretval;
    size_t computed_size;
    hretval = cmox_hash_compute(CMOX_SHA256_ALGO, /* Use SHA224 algorithm */
                                data, dataSize,   /* Message to digest */
                                Computed_Hash,    /* Data buffer to receive digest data */
                                CMOX_SHA256_SIZE, /* Expected digest size */
                                &computed_size);  /* Size of computed digest */

    if (hretval != CMOX_HASH_SUCCESS)
    {
        return CL_ResFailed;
    }

    /* Verify generated data size is the expected one */
    if (computed_size != CMOX_SHA256_SIZE)
    {
        return CL_ResFailed;
    }

    cmox_ecc_construct(&Ecc_Ctx, CMOX_ECC256_MATH_FUNCS, Working_Buffer, sizeof(Working_Buffer));

    cmox_ecc_retval_t retval;

    uint32_t fault_check = CMOX_ECC_AUTH_FAIL;
    retval = cmox_ecdsa_verify(&Ecc_Ctx,                        /* ECC context */
                               CMOX_ECC_CURVE_SECP256K1,        /* SECP256R1 ECC curve selected */
                               Public_Key, sizeof(Public_Key),  /* Public key for verification */
                               Computed_Hash, CMOX_SHA256_SIZE, /* Digest to verify */
                               sign, signSize,                  /* Data buffer to receive signature */
                               &fault_check);                   /* Fault check variable: to ensure no fault injection occurs during this API call */

    if (retval != CMOX_ECC_AUTH_SUCCESS)
    {
        return CL_ResFailed;
    }
    /* Verify Fault check variable value */
    if (fault_check != CMOX_ECC_AUTH_SUCCESS)
    {
        return CL_ResFailed;
    }

    /* Cleanup context */
    cmox_ecc_cleanup(&Ecc_Ctx);
    return CL_ResSuccess;
}
