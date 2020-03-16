/*
 * aes_decrypt_driver.h
 *
 *  Created on: Feb 19, 2020
 *      Author: heiko
 */

#ifdef SRC_AES_AES_DECRYPT_DRIVER_H_
#define SRC_AES_AES_DECRYPT_DRIVER_H_

#include "xparameters.h"
#include "xintc.h"

#define AES_KEYLEN 16   // Key length in bytes
#define AES_keyExpSize 176

#define AES_DECRYPT_BASEADDR XPAR_AES_DECRYPT_2_0_0_BASEADDR
#define COMMAND_REGISTER_OFFSET 0X00
#define STATUS_REGISTER_OFFSET 0X04
#define BASE_KEY_REGISTERS_OFFSET 0X08
#define BASE_DATA_IN_REGISTER_OFFSET 0xB8
#define BASE_DATA_OUT_REGISTER_OFFSET 0xC8

void start_decryption();

u32 is_decryption_complete();

void load_key(u8 * RoundKey);

void load_data_block(u32 * block);

void load_data_block8(u8 * block);

void read_data_block(u32 * block);

void read_data_block8(u8 * block);

void AES_decrypt_block(u8* buf);

#endif /* SRC_AES_AES_DECRYPT_DRIVER_H_ */
