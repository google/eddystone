/* mbed Microcontroller Library
 * Copyright (c) 2006-2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "../EntropySource.h"

#if defined(TARGET_NRF51822) || defined(TARGET_MCU_NRF52832) /* Persistent storage supported on nrf51 platforms */

#include "nrf_soc.h"
#include "nrf_error.h"
#include "mbed.h"
#include <mbedtls/entropy.h>

/*
 * nRF51 has a TRNG that we can access using SoftDevice.
 */
int eddystoneEntropyPoll(void *data, unsigned char *output, size_t len, size_t *olen)
{
    uint8_t bytes_available = 0;

    // get the number of random bytes available
    if (sd_rand_application_bytes_available_get(&bytes_available) != NRF_SUCCESS) {
        return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    }

    // if there is more bytes available that what is requested,
    // truncate the number of bytes in output to len, otherwise use the total
    // of bytes available.
    const uint8_t output_len = bytes_available > len ? len : bytes_available;

    if (output_len) {
        // transfer "output_len" random bytes to output.
        if (sd_rand_application_vector_get(output, output_len) != NRF_SUCCESS) {
            return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
        }
    }

    // Everything went fine, commit the output_len to the output parameter
    *olen = output_len;
    return 0;
}

int eddystoneRegisterEntropySource(	mbedtls_entropy_context* ctx) {
    uint8_t pool_capacity;
    sd_rand_application_pool_capacity_get(&pool_capacity);

    return mbedtls_entropy_add_source(
        ctx,
        eddystoneEntropyPoll, // entropy source function
        NULL,                 // entropy source data, NULL in this case
        pool_capacity,        //  minimum number of bytes the entropy pool should wait on from this callback before releasing entropy
        MBEDTLS_ENTROPY_SOURCE_STRONG
    );
}

#endif /* #ifdef TARGET_NRF51822 */
