
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
#include "../frankencrypt.h"


#ifdef SHA256_H
#include <string.h>
#include "sha256.h"
#include "xil_io.h"



//static int last_init = 0;
//static int last_next = 0;
#include "../util.h"
#include "xparameters.h"
#include <stdio.h>

u32 *led = (u32*) XPAR_RGB_PWM_0_PWM_AXI_BASEADDR;
const struct color RED =    {0x01ff, 0x0000, 0x0000};
const struct color YELLOW = {0x01ff, 0x01ff, 0x0000};
const struct color GREEN =  {0x0000, 0x01ff, 0x0000};
const struct color BLUE =   {0x0000, 0x0000, 0x01ff};

#define change_state(state, color) setLED(led, color);
#define set_stopped() change_state(STOPPED, RED)
#define set_working() change_state(WORKING, YELLOW)
#define set_playing() change_state(PLAYING, GREEN)
#define set_paused()  change_state(PAUSED, BLUE)

int sha256_digest_complete(){
	return (Xil_In32(SHA_BASEADDR + SHA_STATUS_OFFSET) & 2) >> 1 ;
}

int sha256_ready(){
	return Xil_In32(SHA_BASEADDR + SHA_STATUS_OFFSET) & 1;
}

void sha256_trigger_init()
{
	//while(sha256_ready()){
	//while(!sha256_ready()){} // wait for digest to complete
		//set_stopped()
		int current_value = Xil_In32(SHA_BASEADDR + SHA_COMMAND_OFFSET);
		current_value = current_value ^ 2; //Toggle bit
		Xil_Out32(SHA_BASEADDR + SHA_COMMAND_OFFSET, current_value);
	//}
	//set_playing();
}

void sha256_trigger_next()
{
	//while(sha256_ready()){
	//while(!sha256_ready()){} //wait for digest to complete
		//set_working()
		int current_value = Xil_In32(SHA_BASEADDR + SHA_COMMAND_OFFSET);
		current_value = current_value ^ 1; //Toggle bit
		Xil_Out32(SHA_BASEADDR + SHA_COMMAND_OFFSET, current_value);
	//}
	//set_playing();
}

void pad(uint8_t * data, size_t len)
{

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
//	mb_printf("Digesing Block \n\r");
	while(!sha256_ready()){/*set_paused();*/} //Wait for digest to complete
//	mb_printf("Last Digest Complete \n\r");
	//set_playing();
	load_block(block);
//	mb_printf("What is happening? \n\r");
//	for( int i = 0; i < 64; i++){
//		mb_printf("%02x", block[i]);
//	}mb_printf("\r\n");

	if(init){
		sha256_trigger_init();
	}else{
		sha256_trigger_next();
	}
}

// data is buffer, datalen is length of data, init = 0 continue hash, init = 1 start new hash
void sha256_compute_hash(uint8_t * data, uint32_t len, uint8_t init, uint8_t * hash_out)
{
	//enableLED(led);

	uint8_t buffer[64] = {0};

	unsigned long bitlen = 0;
	int i =0;
	int datalen = 0;
	for (i = 0; i < len; ++i) {
		buffer[datalen] = data[i];
		datalen++;
		if (datalen == 64) {
			digest_block(buffer, init);
			init = 0;
			bitlen += 512;
			datalen = 0;
		}
	}

	//uint32_t i;
	i = datalen;

	// Pad whatever data is left in the buffer.
	if (datalen < 56) {
		buffer[i++] = 0x80;
		while (i < 56)
			buffer[i++] = 0x00;
	}
	else {
		buffer[i++] = 0x80;
		while (i < 64)
			buffer[i++] = 0x00;
		digest_block(buffer, init);
		init = 0;
		memset(buffer, 0, 56);
	}

	bitlen += datalen*8;
	buffer[63] = bitlen ;
	buffer[62] = bitlen >> 8;
	buffer[61] = bitlen >> 16;
	buffer[60] = bitlen >> 24;
	buffer[59] = 0;
	buffer[58] = 0;
	buffer[57] = 0;
	buffer[56] = 0;
	//We never have lengths greater then 32 bit integers. So don't pad more


	digest_block(buffer, init);

	read_digest(hash_out);
}
#endif
