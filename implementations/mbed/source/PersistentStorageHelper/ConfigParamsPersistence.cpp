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

#include "ConfigParamsPersistence.h"

#if !defined(TARGET_NRF51822) && !defined(TARGET_NRF52832) /* Persistent storage supported on nrf51 platforms */
    /**
     * When not using an nRF51-based target then persistent storage is not available.
     */
    #warning "EddystoneService is not configured to store configuration data in non-volatile memory"

    bool loadEddystoneServiceConfigParams(EddystoneService::EddystoneParams_t *paramsP)
    {
        /* Avoid compiler warnings */
        (void) paramsP;

        /*
         * Do nothing and let the main program set Eddystone params to
         * defaults
         */
        return false;
    }

    void saveEddystoneServiceConfigParams(const EddystoneService::EddystoneParams_t *paramsP)
    {
        /* Avoid compiler warnings */
        (void) paramsP;

        /* Do nothing... */
        return;
    }

    void saveEddystoneTimeParams(const TimeParams_t *timeP)
    {
        /* Avoid compiler warnings */
        (void) timeP;

        /* Do nothing... */
        return;
    }

#endif /* #ifdef TARGET_NRF51822 */
