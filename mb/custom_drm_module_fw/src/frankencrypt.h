/*
 * frankencrypt.h
 *
 *  Created on: Mar 4, 2020
 *      Author: heiko
 */

#ifndef SRC_FRANKENCRYPT_H_
#define SRC_FRANKENCRYPT_H_
#include "xintc.h"
#include "constants.h"


#define SHA256_HASH_SZ 32
#define SHA256_BLOCK_SIZE 64

// AES Functions
void load_key(u8 * RoundKey);

void AES_decrypt_block(u8* buf);

// SHA Function
void sha256_compute_hash(uint8_t * data, uint32_t datalen, uint8_t init, uint8_t * hash_out);

// RSA Function
void rsa_encrypt(uint8_t * m, uint8_t * key, uint8_t * n,  uint8_t * out);

void rsa_get_verify_out(uint8_t * out);

#endif /* SRC_FRANKENCRYPT_H_ */
