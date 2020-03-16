/*
 * aes_decrypt_driver.c
 *
 *  Created on: Feb 19, 2020
 *      Author: heiko
 */
#include "aes_decrypt_driver.h"

#include "xil_io.h"


void start_decryption()
{
	Xil_Out32(AES_DECRYPT_BASEADDR + COMMAND_REGISTER_OFFSET,0);
	Xil_Out32(AES_DECRYPT_BASEADDR + COMMAND_REGISTER_OFFSET,1);
}

u32 is_decryption_complete()
{
	return Xil_In32(AES_DECRYPT_BASEADDR + STATUS_REGISTER_OFFSET);
}


void load_key(u8 * RoundKey)
{
	for (int i = 0; i < AES_keyExpSize; i+=4)
	{
		uint32_t * key_word = (u32*)(RoundKey + i); //(RoundKey[i]<<24) | (RoundKey[i+1]<<16) | (RoundKey[i+2]<<8) | RoundKey[i+3];
		Xil_Out32BE(AES_DECRYPT_BASEADDR + BASE_KEY_REGISTERS_OFFSET + i, (*key_word));
	}
}

// Block[3] is MSB Block[0] is MSB
void load_data_block(u32 * block)
{
	Xil_Out32(AES_DECRYPT_BASEADDR + BASE_DATA_IN_REGISTER_OFFSET,block[0]);
	Xil_Out32(AES_DECRYPT_BASEADDR + BASE_DATA_IN_REGISTER_OFFSET + 4,block[1]);
	Xil_Out32(AES_DECRYPT_BASEADDR + BASE_DATA_IN_REGISTER_OFFSET + 8,block[2]);
	Xil_Out32(AES_DECRYPT_BASEADDR + BASE_DATA_IN_REGISTER_OFFSET + 12,block[3]);
}

// Block[3] is MSB Block[0] is MSB
void read_data_block(u32 * block)
{
	block[0] = Xil_In32(AES_DECRYPT_BASEADDR + BASE_DATA_OUT_REGISTER_OFFSET);
	block[1] = Xil_In32(AES_DECRYPT_BASEADDR + BASE_DATA_OUT_REGISTER_OFFSET + 4);
	block[2] = Xil_In32(AES_DECRYPT_BASEADDR + BASE_DATA_OUT_REGISTER_OFFSET + 8);
	block[3] = Xil_In32(AES_DECRYPT_BASEADDR + BASE_DATA_OUT_REGISTER_OFFSET + 12);
}


void load_data_block8(u8 * block)
{
	for (int i = 0; i < 16; i+=4)
	{
		Xil_Out32BE(AES_DECRYPT_BASEADDR + BASE_DATA_IN_REGISTER_OFFSET + i, *(u32*)(block + i));
	}
}

void read_data_block8(u8 * block)
{
	for (int i = 0; i < 16; i+=4)
	{
		*(u32*)(block+i) = Xil_In32BE(AES_DECRYPT_BASEADDR + BASE_DATA_OUT_REGISTER_OFFSET + i);
	}
}

