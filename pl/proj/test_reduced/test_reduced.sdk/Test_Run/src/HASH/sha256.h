/*********************************************************************
* Filename:   sha256.h
* Author:     Brad Conte (brad AT bradconte.com)
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding SHA1 implementation.
*********************************************************************/

#ifndef SHA256_H
#define SHA256_H

#include <stdint.h>
#include "xparameters.h"

#define SHA_BASEADDR XPAR_AXI_SHA256_V1_0_0_BASEADDR
#define SHA_COMMAND_OFFSET 0
#define SHA_STATUS_OFFSET 4
#define SHA_BLOCK_OFFSET 8
#define SHA_DIGEST_OFFSET 72

#define SHA256_HASH_SZ 32
#define SHA256_BLOCK_SIZE 64

// data is buffer, datalen is length of data, init = 0 continue hash, init = 1 start new hash
// if datalen is not in complete 512 bit blocks this function will automatically pad it
// hash_out must be 32 bytes
void sha256_compute_hash(uint8_t * data, uint32_t datalen, uint8_t init, uint8_t * hash_out);

#endif   // SHA256_H
