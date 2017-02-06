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
#include "EntropySource.h"

#if !defined(TARGET_NRF51822) && !defined(TARGET_MCU_NRF52832) /* Persistent storage supported on nrf51 platforms */
    /**
     * When not using an nRF51-based target then entropy source is currently unimplemented.
     */
    #error "INSECURE CONFIGURATION - YOU MUST IMPLEMENT AN ENTROPY SOURCE"

    int eddystoneRegisterEntropySource(	mbedtls_entropy_context* ctx) { 
      return 1;
    }

    int eddystoneEntropyPoll( void *data,
                        unsigned char *output, size_t len, size_t *olen )
    {
        return( 1 );
    }
#endif /* #ifdef TARGET_NRF51822 */
