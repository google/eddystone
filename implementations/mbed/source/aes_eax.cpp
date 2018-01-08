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
 
#include <string.h>

// #include "aes_eax.h"
// set defines before loading aes.h
#define MBEDTLS_CIPHER_MODE_CBC
#define MBEDTLS_CIPHER_MODE_CTR
#include "mbedtls/aes.h"

#define EDDY_ERR_EAX_AUTH_FAILED    -0x000F /**< Authenticated decryption failed. */

void gf128_double_( unsigned char val[16] )
{
	int i;
	int carry = val[0] >> 7;
	int xv = (-carry) & 0x87;
	for (i = 15; i >= 0; i--) {
		carry = val[i] >> 7;
		val[i] = (val[i] << 1) ^ xv;
		xv = carry;
	}
}

int compute_cmac_( mbedtls_aes_context *ctx,
		          const unsigned char *input,
		          size_t length,
		          unsigned char param,
		          unsigned char mac[16] )
{
	unsigned char buf[16], iv[16];
	memset(buf, 0, sizeof(buf));
	buf[15] = param;
	memset(iv, 0, sizeof(iv));
	length += 16;

	unsigned char pad[16];
	memset(pad, 0, sizeof(pad));
	mbedtls_aes_crypt_ecb(ctx, MBEDTLS_AES_ENCRYPT, pad, pad);
	gf128_double_(pad);
	if (length & 15) {
		gf128_double_(pad);
		pad[length & 15] ^= 0x80;
	}

	const unsigned char *tmp_input = buf;
	while (length > 16) {
		mbedtls_aes_crypt_cbc(ctx, MBEDTLS_AES_ENCRYPT, 16, iv, tmp_input, buf);
		if (tmp_input == buf) {
			tmp_input = input;
		} else {
			tmp_input += 16;
		}
		length -= 16;
	}

	size_t i;
	for (i = 0; i < length; i++)
		pad[i] ^= tmp_input[i];

	mbedtls_aes_crypt_cbc(ctx, MBEDTLS_AES_ENCRYPT, 16, iv, pad, mac);
	return 0;
}

int eddy_aes_authcrypt_eax( mbedtls_aes_context *ctx,
                            int mode,                   
                            const unsigned char *nonce, 
                            size_t nonce_length,        
                            const unsigned char *header,
                            size_t header_length, 
                            size_t message_length,
                            const unsigned char *input,
                            unsigned char *output,
                            unsigned char *tag,
                            size_t tag_length )
{
	unsigned char header_mac[16];
	unsigned char nonce_mac[16];
	unsigned char ciphertext_mac[16];
	uint8_t i;
	compute_cmac_(ctx, header, header_length, 1, header_mac);
	compute_cmac_(ctx, nonce, nonce_length, 0, nonce_mac);
	if (mode == MBEDTLS_AES_DECRYPT) {
		compute_cmac_(ctx, input, message_length, 2, ciphertext_mac);
		unsigned char n_ok = 0;
		for (i = 0; i < tag_length; i++) {
			ciphertext_mac[i] ^= header_mac[i];
			ciphertext_mac[i] ^= nonce_mac[i];
			ciphertext_mac[i] ^= tag[i];
			n_ok |= ciphertext_mac[i];
		}
		if (n_ok)
			return EDDY_ERR_EAX_AUTH_FAILED;
	}
	size_t nc_off = 0;
	unsigned char nonce_copy[16];
	memcpy(nonce_copy, nonce_mac, sizeof(nonce_mac));
	unsigned char sb[16];
	mbedtls_aes_crypt_ctr(ctx, message_length, &nc_off, nonce_copy, sb, input, output);
	if (mode == MBEDTLS_AES_ENCRYPT) {
		compute_cmac_(ctx, output, message_length, 2, ciphertext_mac); 
		for (i = 0; i < tag_length; i++)
			tag[i] = header_mac[i] ^ nonce_mac[i] ^ ciphertext_mac[i];
	}
	return 0;
}
