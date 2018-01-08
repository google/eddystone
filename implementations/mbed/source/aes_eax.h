#if !defined(AES_EAX_H__INCLUDED__)
#define AES_EAX_H__INCLUDED__

#define MBEDTLS_CIPHER_MODE_CBC
/*
 * Copyright (c) 2016, Google Inc, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#define MBEDTLS_CIPHER_MODE_CTR
#include "mbedtls/aes.h"

int compute_cmac_( mbedtls_aes_context *ctx,
		          const unsigned char *input,
		          size_t length,
		          unsigned char param,
		          unsigned char mac[16] );
		          
void gf128_double_( unsigned char val[16] );   

int eddy_aes_authcrypt_eax( mbedtls_aes_context *ctx,
                            int mode,                       /* ENCRYPT/DECRYPT */
                            const unsigned char *nonce,     /* 48-bit nonce */ 
                            size_t nonce_length,            /* = 6 */
                            const unsigned char *header,    /* Empty buffer */
                            size_t header_length,           /* = 0 */
                            size_t message_length,          /* Length of input & output buffers 12 */
                            const unsigned char *input,
                            unsigned char *output,
                            unsigned char *tag,
                            size_t tag_length );            /* = 2 */
                            
                        

#endif /* defined(AES_EAX_H__INCLUDED__) */