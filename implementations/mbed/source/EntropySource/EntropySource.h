#ifndef __ENTROPY_SOURCE_H__
#define __ENTROPY_SOURCE_H__
#include <stddef.h>
#include <mbedtls/entropy.h>

int eddystoneRegisterEntropySource(	mbedtls_entropy_context* ctx);

int eddystoneEntropyPoll( void *data,
                    unsigned char *output, size_t len, size_t *olen );

#endif // __ENTROPY_SOURCE_H__
