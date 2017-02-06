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

#ifndef __BLE_CONFIG_PARAMS_PERSISTENCE_H__
#define __BLE_CONFIG_PARAMS_PERSISTENCE_H__

#include "../EddystoneService.h"

/**
 * Generic API to load the Eddystone Service configuration parameters from persistent
 * storage. If persistent storage isn't available, the persistenceSignature
 * member of params may be left un-initialized to the MAGIC, and this will cause
 * a reset to default values.
 *
 * @param[out] paramsP
 *                 The parameters to be filled in from persistence storage. This
 *                 argument can be NULL if the caller is only interested in
 *                 discovering the persistence status of params.
 *
 * @return true if params were loaded from persistent storage and have usefully
 *         initialized fields.
 */
bool loadEddystoneServiceConfigParams(EddystoneService::EddystoneParams_t *paramsP);

/**
 * Generic API to store the Eddystone Service configuration parameters to persistent
 * storage. It typically initializes the persistenceSignature member of the
 * params to the MAGIC value to indicate persistence.
 *
 * @param[in,out] paramsP
 *                    The params to be saved; persistenceSignature member gets
 *                    updated if persistence is successful.
 *
 * @note The save operation may be asynchronous. It may be a short while before
 *       the request takes affect. Reading back saved configParams may not yield
 *       correct behaviour if attempted soon after a store.
 */
void saveEddystoneServiceConfigParams(const EddystoneService::EddystoneParams_t *paramsP);

/**
 * Generic API to store the Eddystone TimeParams (a subset of Config Params) for 
 * speed/power efficiency.
 *
 * @param[in,out] timeP
 *                    The params to be saved; persistenceSignature member gets
 *                    updated if persistence is successful.
 *
 * @note The save operation may be asynchronous. It may be a short while before
 *       the request takes affect. Reading back saved configParams may not yield
 *       correct behaviour if attempted soon after a store.
 */
void saveEddystoneTimeParams(const TimeParams_t *timeP);

#endif /* #ifndef __BLE_CONFIG_PARAMS_PERSISTENCE_H__*/
