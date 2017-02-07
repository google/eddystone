/*
 * PackageLicenseDeclared: Apache-2.0
 * Copyright (c) 2015 ARM Limited
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

#ifndef __MBED_UTIL_CRITICAL_SECTION_LOCK_H__
#define __MBED_UTIL_CRITICAL_SECTION_LOCK_H__

#include <stdint.h>
#include "cmsis.h"

namespace mbed {
namespace util {

/** RAII object for disabling, then restoring, interrupt state
  * Usage:
  * @code
  *
  * void f() {
  *     // some code here
  *     {
  *         CriticalSectionLock lock;
  *         // Code in this block will run with interrupts disabled
  *     }
  *     // interrupts will be restored to their previous state
  * }
  * @endcode
  */
class CriticalSectionLock {
public:
    CriticalSectionLock() {
        _state = __get_PRIMASK();
        __disable_irq();
    }

    ~CriticalSectionLock() {
        __set_PRIMASK(_state);
    }

private:
    uint32_t _state;
};

} // namespace util
} // namespace mbed

#endif // #ifndef __MBED_UTIL_CRITICAL_SECTION_LOCK_H__
