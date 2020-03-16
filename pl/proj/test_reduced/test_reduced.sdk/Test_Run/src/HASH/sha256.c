/*********************************************************************
* Filename:   sha256.c
* Author:     Brad Conte (brad AT bradconte.com)
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Implementation of the SHA-256 hashing algorithm.
              SHA-256 is one of the three algorithms in the SHA2
              specification. The others, SHA-384 and SHA-512, are not
              offered in this implementation.
              Algorithm specification can be found here:
               * http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
              This implementation uses little endian byte order.
*********************************************************************/

/*************************** HEADER FILES ***************************/

#include <string.h>
#include "sha256.h"
#include "xil_io.h"



//static int last_init = 0;
//static int last_next = 0;


void sha256_trigger_init()
{
	int current_value = Xil_In32(SHA_BASEADDR + SHA_COMMAND_OFFSET);
	current_value = current_value ^ 2; //Toggle bit
	Xil_Out32(SHA_BASEADDR + SHA_COMMAND_OFFSET, current_value);
}

void sha256_trigger_next()
{
	int current_value = Xil_In32(SHA_BASEADDR + SHA_COMMAND_OFFSET);
	current_value = current_value ^ 2; //Toggle bit
	Xil_Out32(SHA_BASEADDR + SHA_COMMAND_OFFSET, current_value);
}

int sha256_digest_complete(){
	return (Xil_In32(SHA_BASEADDR + SHA_STATUS_OFFSET) & 2) >> 1 ;
}

int sha256_ready(){
	return Xil_In32(SHA_BASEADDR + SHA_STATUS_OFFSET) & 1;
}

void pad(uint8_t * data, size_t len)
{
	uint32_t i;
	i = len;

	// Pad whatever data is left in the buffer.
	if (len < 56) {
		data[i++] = 0x80;
		while (i < 56)
			data[i++] = 0x00;
	}
	else {
		data[i++] = 0x80;
		while (i < 64)
			data[i++] = 0x00;
		//sha256_transform(ctx, ctx->data);
		//memset(ctx->data, 0, 56);
	}
}

//must be len 64
void load_block(uint8_t * block)
{
	//Block size is 64 bytes - 512 bits
	for (int i = 0; i < 64; i+=4)
	{
		Xil_Out32BE(SHA_BASEADDR + SHA_BLOCK_OFFSET + i, *(u32*)(block + i));
	}
}

void read_digest(u8 * digest_buf)
{
	//Digest (HASH) size is 32 bytes - 256 bits
	while(!sha256_digest_complete()){} //Wait for digest to complete
	for (int i = 0; i < SHA256_HASH_SZ; i+=4)
	{
		*(u32*)(digest_buf+i) = Xil_In32BE(SHA_BASEADDR + SHA_DIGEST_OFFSET + i);
	}
}

void digest_block(uint8_t * block, uint8_t init)
{

	load_block(block);
	while(!sha256_ready()){} //Wait for digest to complete

	if(init){
		sha256_trigger_init();
	}else{
		sha256_trigger_next();
	}
}

// data is buffer, datalen is length of data, init = 0 continue hash, init = 1 start new hash
void sha256_compute_hash(uint8_t * data, uint32_t datalen, uint8_t init, uint8_t * hash_out)
{
	uint32_t bytes_left = datalen;
	uint8_t buffer[512] = {0};
	while(bytes_left >= 64) //While complete blocks are left
	{
		memcpy(buffer,data,64);
		data+=64;
		bytes_left-=64;
		digest_block(buffer, init);
		init = 0; //init must only be done once
	}
	memcpy(buffer,data,bytes_left);
	pad( data, bytes_left);
	digest_block(buffer, init);

	read_digest(hash_out);
}