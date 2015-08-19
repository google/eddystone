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

#include "pstorage.h"
#include "nrf_error.h"
#include "ConfigParamsPersistence.h"

/**
 * Nordic specific structure used to store params persistently.
 * It extends EddystoneURLConfigService::Params_t with a persistence signature.
 */
struct PersistentParams_t {
    EddystoneURLConfigService::Params_t params;
    uint32_t                         persistenceSignature; /* This isn't really a parameter, but having the expected
                                                            * magic value in this field indicates persistence. */

    static const uint32_t MAGIC = 0x1BEAC000;              /* Magic that identifies persistence */
};

/**
 * The following is a module-local variable to hold configuration parameters for
 * short periods during flash access. This is necessary because the pstorage
 * APIs don't copy in the memory provided as data source. The memory cannot be
 * freed or reused by the application until this flash access is complete. The
 * load and store operations in this module initialize persistentParams and then
 * pass it on to the 'pstorage' APIs.
 */
static PersistentParams_t persistentParams;

static pstorage_handle_t pstorageHandle;

/**
 * Dummy callback handler needed by Nordic's pstorage module. This is called
 * after every flash access.
 */
static void pstorageNotificationCallback(pstorage_handle_t *p_handle,
                                         uint8_t            op_code,
                                         uint32_t           result,
                                         uint8_t           *p_data,
                                         uint32_t           data_len)
{
    /* APP_ERROR_CHECK(result); */
}

/* Platform-specific implementation for persistence on the nRF5x. Based on the
 * pstorage module provided by the Nordic SDK. */
bool loadEddystoneURLConfigParams(EddystoneURLConfigService::Params_t *paramsP)
{
    static bool pstorageInitied = false;
    if (!pstorageInitied) {
        pstorage_init();

        static pstorage_module_param_t pstorageParams = {
            .cb          = pstorageNotificationCallback,
            .block_size  = sizeof(PersistentParams_t),
            .block_count = 1
        };
        pstorage_register(&pstorageParams, &pstorageHandle);
        pstorageInitied = true;
    }

    if ((pstorage_load(reinterpret_cast<uint8_t *>(&persistentParams), &pstorageHandle, sizeof(PersistentParams_t), 0) != NRF_SUCCESS) ||
        (persistentParams.persistenceSignature != PersistentParams_t::MAGIC)) {
        // On failure zero out and let the service reset to defaults
        memset(paramsP, 0, sizeof(EddystoneURLConfigService::Params_t));
        return false;
    }

    memcpy(paramsP, &persistentParams.params, sizeof(EddystoneURLConfigService::Params_t));
    return true;
}

/* Platform-specific implementation for persistence on the nRF5x. Based on the
 * pstorage module provided by the Nordic SDK. */
void saveEddystoneURLConfigParams(const EddystoneURLConfigService::Params_t *paramsP)
{
    memcpy(&persistentParams.params, paramsP, sizeof(EddystoneURLConfigService::Params_t));
    if (persistentParams.persistenceSignature != PersistentParams_t::MAGIC) {
        persistentParams.persistenceSignature = PersistentParams_t::MAGIC;
        pstorage_store(&pstorageHandle,
                       reinterpret_cast<uint8_t *>(&persistentParams),
                       sizeof(PersistentParams_t),
                       0 /* offset */);
    } else {
        pstorage_update(&pstorageHandle,
                        reinterpret_cast<uint8_t *>(&persistentParams),
                        sizeof(PersistentParams_t),
                        0 /* offset */);
    }
}
