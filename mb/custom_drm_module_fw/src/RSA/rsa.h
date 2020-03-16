#ifndef RSA_H_
#define RSA_H_

//#include "../constants.h"
//#include "../secrets.h"
#include <stdint.h>

int verify_signature(uint8_t * signature, uint8_t * public_key_n);

void rsa_encrypt(uint8_t * m, uint8_t * key, uint8_t * n,  uint8_t * out);

void rsa_begin_verify(uint8_t * m, uint8_t * key, uint8_t * n);

void rsa_get_verify_out(uint8_t * out);

#endif
