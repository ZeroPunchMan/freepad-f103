#include "sign_check.h"
#include "cmox_crypto.h"
#include "main.h"

cmox_ecc_handle_t Ecc_Ctx;
/* ECC working buffer */
uint8_t Working_Buffer[2000];

/* Random data buffer */
uint8_t Computed_Random[32];
#define FAILED ((TestStatus)0)
#define PASSED ((TestStatus)1)
typedef uint32_t TestStatus;
volatile TestStatus glob_status = FAILED;
uint8_t Computed_Hash[CMOX_SHA256_SIZE] = {0};
uint8_t Computed_Signature[CMOX_ECC_SECP256K1_SIG_LEN] =
    {
        0};

const uint8_t Public_Key[] = {
    0x86, 0x67, 0xb2, 0xfb, 0xf8, 0x79, 0x76, 0x12,
    0x22, 0xb0, 0x30, 0x6e, 0xc6, 0x54, 0x0d, 0x2c,
    0x83, 0x83, 0x8b, 0xad, 0x47, 0x4f, 0xc7, 0x03,
    0x17, 0x33, 0x8f, 0xeb, 0x97, 0xe0, 0x69, 0xbb,
    0xa9, 0x68, 0xcd, 0x3a, 0x9c, 0xe4, 0x5e, 0x5c,
    0x3b, 0x76, 0xc6, 0xf9, 0xc1, 0x06, 0x14, 0xa8,
    0xdf, 0x4d, 0xbd, 0xab, 0x6d, 0x5e, 0x30, 0xf5,
    0xab, 0x0d, 0x97, 0xaf, 0x5e, 0x6a, 0xb7, 0xeb};

uint8_t Computed_Hash[CMOX_SHA256_SIZE];
uint8_t Computed_Signature[CMOX_ECC_SECP256R1_SIG_LEN];

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
                               &fault_check);                   /* Fault check variable:
                                                    to ensure no fault injection occurs during this API call */

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
