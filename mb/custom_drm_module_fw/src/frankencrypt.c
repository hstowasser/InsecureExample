/*
 * frankencrypt.c
 *
 *  Created on: Mar 4, 2020
 *      Author: heiko
 */
#include "frankencrypt.h"
#include "xparameters.h"
#include "stdio.h"
//#include "secrets.h"

#define FRANKENCRYPT_BASE XPAR_FRANKENCRYPTCORE_0_BASEADDR

#define FRANKENCRYPT_STATUS_REGISTER (FRANKENCRYPT_BASE)
#define FRANKENCRYPT_COMMAND_REGISTER (FRANKENCRYPT_BASE + 4)
#define FRANKENCRYPT_ADDRESS_REGISTER (FRANKENCRYPT_BASE + 8)
#define FRANKENCRYPT_DATA_REGISTER (FRANKENCRYPT_BASE + 12)

#define USE_AES (0x40000000)
#define USE_SHA (0x80000000)
#define USE_RSA (0xC0000000)

#define TRIGGER_AES (USE_AES | 1)
#define TRIGGER_SHA256_INIT (USE_SHA | 1)
#define TRIGGER_SHA256_NEXT (USE_SHA)
#define TRIGGER_RSA (USE_RSA | 1)

#define FRANKENCRYPT_AES_STATUS_MASK (1)
#define FRANKENCRYPT_SHA_CORE_READY_STATUS_MASK (2)
#define FRANKENCRYPT_SHA_DIGEST_VALID_STATUS_MASK (4)
#define FRANKENCRYPT_RSA_STATUS_MASK (8)

#define FRANKENCRYPT_ADDRESS_AES_DATA_IN (44) //MSB
#define FRANKENCRYPT_ADDRESS_AES_DATA_OUT (48)

#define FRANKENCRYPT_ADDRESS_SHA_DATA_OUT (44) //MSB
#define FRANKENCRYPT_ADDRESS_RSA_DATA_OUT (96)

// ------------------------------------ AES --------------------------------------------------
#define AES_keyExpSize 176
static u8 key_buffer[AES_keyExpSize];
static int Last_Module_Used = 0;

void load_key(u8 * RoundKey)
{
	// Save the key for later
	for (int i = 0; i < AES_keyExpSize; i++){
		key_buffer[i] = RoundKey[i];
	}
	//Setup address
	Xil_Out32(FRANKENCRYPT_ADDRESS_REGISTER, 0); // Key starts at 0

	for (int i = 0; i < AES_keyExpSize; i+=4)
	{
		uint32_t * key_word = (u32*)(RoundKey + i);
		Xil_Out32BE(FRANKENCRYPT_DATA_REGISTER, (*key_word)); //Address will auto increment
	}
}

void reload_key()
{
	//Setup address
	Xil_Out32(FRANKENCRYPT_ADDRESS_REGISTER, 0); // Key starts at 0
	for (int i = 0; i < AES_keyExpSize; i+=4)
	{
		uint32_t * key_word = (u32*)(key_buffer + i);
		Xil_Out32BE(FRANKENCRYPT_DATA_REGISTER, (*key_word)); //Address will auto increment
	}
}


void load_data_block8(u8 * block)
{
	//Setup address
	Xil_Out32(FRANKENCRYPT_ADDRESS_REGISTER, FRANKENCRYPT_ADDRESS_AES_DATA_IN);
	for (int i = 0; i < 16; i+=4)
	{
		uint32_t * data_word = (u32*)(block + i);
		//printf("%x \r\n", *data_word);
		Xil_Out32BE(FRANKENCRYPT_DATA_REGISTER, (*data_word));
	}
}

void read_data_block8(u8 * block)
{
	//Setup address
	Xil_Out32(FRANKENCRYPT_ADDRESS_REGISTER, FRANKENCRYPT_ADDRESS_AES_DATA_OUT);
	for (int i = 0; i < 16; i+=4)
	{
		*(u32*)(block+i) = Xil_In32BE(FRANKENCRYPT_DATA_REGISTER);
	}
}

void start_decryption()
{
	Xil_Out32(FRANKENCRYPT_COMMAND_REGISTER, TRIGGER_AES); // Send decrypt command
}

u32 is_decryption_complete()
{
	return (Xil_In32(FRANKENCRYPT_STATUS_REGISTER) & FRANKENCRYPT_AES_STATUS_MASK) > 0; //If first bit is high, decryption is complete
}

void AES_decrypt_block(u8* buf)
{
	if(Last_Module_Used != USE_AES){ //If we used the other interfaces our key could have been overwritten
		reload_key();
	}
	Last_Module_Used = USE_AES;

	load_data_block8(buf); //Not sure if this cast will work
    start_decryption();

	while(is_decryption_complete() == 0){}

	read_data_block8(buf); //Not sure if this cast will work
}



// ------------------------------------------------- SHA 256 -------------------------------------------------------




int sha256_digest_complete(){
	volatile u32 status = Xil_In32(FRANKENCRYPT_STATUS_REGISTER);
	//mb_printf("Digest Ready %x \n\r", status);
	return (status & FRANKENCRYPT_SHA_DIGEST_VALID_STATUS_MASK) > 0;
}

int sha256_ready(){
	volatile u32 status = Xil_In32(FRANKENCRYPT_STATUS_REGISTER);
	//mb_printf("Status Ready %x \n\r", status);
	return (status & FRANKENCRYPT_SHA_CORE_READY_STATUS_MASK) > 0;
}

void sha256_trigger_init()
{
	//printf("Trigger Init %x \n\r", TRIGGER_SHA256_INIT);
	Xil_Out32(FRANKENCRYPT_COMMAND_REGISTER, TRIGGER_SHA256_INIT);
}

void sha256_trigger_next()
{
	Xil_Out32(FRANKENCRYPT_COMMAND_REGISTER, TRIGGER_SHA256_NEXT);
}


//must be len 64
void load_block(uint8_t * block)
{
	//Block size is 64 bytes - 512 bits
	Xil_Out32(FRANKENCRYPT_ADDRESS_REGISTER, 0); // Setup Address
	for (int i = 0; i < 64; i+=4)
	{
		//printf("Address wr %d \r\n", Xil_In32(FRANKENCRYPT_ADDRESS_REGISTER));
		Xil_Out32BE(FRANKENCRYPT_DATA_REGISTER, *(u32*)(block + i));
	}
}

void read_digest(u8 * digest_buf)
{
	//Digest (HASH) size is 32 bytes - 256 bits
	while(!sha256_digest_complete()){} //Wait for digest to complete
	Xil_Out32(FRANKENCRYPT_ADDRESS_REGISTER, FRANKENCRYPT_ADDRESS_SHA_DATA_OUT); //Setup Address
	for (int i = 0; i < SHA256_HASH_SZ; i+=4)
	{
		//printf("Address rd %d \r\n", Xil_In32(FRANKENCRYPT_ADDRESS_REGISTER));
		*(u32*)(digest_buf+i) = Xil_In32BE(FRANKENCRYPT_DATA_REGISTER);
	}
}

void digest_block(uint8_t * block, uint8_t init)
{
	//mb_printf("So confused... \n\r");

	while(!sha256_ready()){ } //Wait for digest to complete
	//mb_printf("Last Digest Complete \n\r");
	//set_playing();
	load_block(block);
//	mb_printf("What is happening? \n\r");
//	for( int i = 0; i < 64; i++){
//		printf("%02x ", block[i]);
//	}printf("\r\n");

	if(init){
		sha256_trigger_init();
	}else{
		sha256_trigger_next();
	}
}

// data is buffer, datalen is length of data, init = 0 continue hash, init = 1 start new hash
void sha256_compute_hash(uint8_t * data, uint32_t len, uint8_t init, uint8_t * hash_out)
{
	//mb_printf("Compute Hash %d \n\r", len);//Debug
	Last_Module_Used = USE_SHA;

	uint8_t buffer[64] = {0};

	unsigned long bitlen = 0;
	int i =0;
	int datalen = 0;
	for (i = 0; i < len; ++i) {
		buffer[datalen] = data[i];
		//mb_printf("data[%d] %02x \r\n", i, data[i]);
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

	read_digest(hash_out); //Reading seems to be stable
//	for( int i = 0; i < 16; i++){
//		mb_printf("%02x ", hash_out[i]);
//	}mb_printf("\r\n");
	//printf("asdf\r\n");
}



// ------------------------------------------------------- RSA --------------------------------------------------------------------------

int rsa_complete()
{
	u32 status = Xil_In32(FRANKENCRYPT_STATUS_REGISTER);
	//printf("RSA Ready %x \n\r", status);
	return (status & FRANKENCRYPT_RSA_STATUS_MASK) > 0;
}

// Currently AES cannot be used while RSA is running. But SHA256 can

// Assumes all inputs are of length RSA_KEY
void rsa_encrypt(uint8_t * m, uint8_t * key, uint8_t * n,  uint8_t * out)
{
	Last_Module_Used = USE_RSA;

	// Load m, n, and key
	Xil_Out32(FRANKENCRYPT_ADDRESS_REGISTER, 0); // Setup Address, e first then M then N
	for (int i = 0; i < RSA_KEY_SZ; i+=4)
	{
		uint32_t word = (key[i]<<24) | (key[i+1]<<16) | (key[i+2]<<8) | key[i+3];
		Xil_Out32(FRANKENCRYPT_DATA_REGISTER, word);
		//Xil_Out32BE(FRANKENCRYPT_DATA_REGISTER, *(u32*)(key + i));
	}
	for (int i = 0; i < RSA_KEY_SZ; i+=4)
	{
		uint32_t word = (m[i]<<24) | (m[i+1]<<16) | (m[i+2]<<8) | m[i+3];
		Xil_Out32(FRANKENCRYPT_DATA_REGISTER, word);
		//Xil_Out32BE(FRANKENCRYPT_DATA_REGISTER, *(u32*)(m + i));
		//mb_printf("%08x \r\n", word);
	}
	for (int i = 0; i < RSA_KEY_SZ; i+=4)
	{
		uint32_t word = (n[i]<<24) | (n[i+1]<<16) | (n[i+2]<<8) | n[i+3];
		Xil_Out32(FRANKENCRYPT_DATA_REGISTER, word);
//		Xil_Out32BE(FRANKENCRYPT_DATA_REGISTER, *(u32*)(n + i));
	}

	//rsa_complete();
	//Trigger RSA
	Xil_Out32(FRANKENCRYPT_COMMAND_REGISTER, TRIGGER_RSA );
	//Reset Trigger
	Xil_Out32(FRANKENCRYPT_COMMAND_REGISTER, TRIGGER_RSA );

	//Wait for RSA to finish.
	while( !rsa_complete() ){}


	//Read out computed stuff
	Xil_Out32(FRANKENCRYPT_ADDRESS_REGISTER, FRANKENCRYPT_ADDRESS_RSA_DATA_OUT); //Setup Address
	for (int i = 0; i < 128; i+=4)
	{
		u32 out_word = Xil_In32(FRANKENCRYPT_DATA_REGISTER);
		out[i] = out_word >> 8*3;
		out[i+1] = out_word >> 8*2;
		out[i+2] = out_word >> 8*1;
		out[i+3] = out_word;
	}

	////Test output
//	Xil_Out32(FRANKENCRYPT_ADDRESS_REGISTER, FRANKENCRYPT_ADDRESS_RSA_DATA_OUT); //Setup Address
//	u32 iout = 0;
//	for (int i = 0; i < 128; i+=4)
//	{
//		iout = Xil_In32(FRANKENCRYPT_DATA_REGISTER);
//		printf("%04x \n\r", iout);
//	}
//	printf("\r\n");
}

void rsa_begin_verify(uint8_t * m, uint8_t * key, uint8_t * n)
{
	Last_Module_Used = USE_RSA;

	// Load m, n, and key
	Xil_Out32(FRANKENCRYPT_ADDRESS_REGISTER, 0); // Setup Address, e first then M then N
	for (int i = 0; i < RSA_KEY_SZ; i+=4)
	{
		uint32_t word = (key[i]<<24) | (key[i+1]<<16) | (key[i+2]<<8) | key[i+3];
		Xil_Out32(FRANKENCRYPT_DATA_REGISTER, word);
	}
	for (int i = 0; i < RSA_KEY_SZ; i+=4)
	{
		uint32_t word = (m[i]<<24) | (m[i+1]<<16) | (m[i+2]<<8) | m[i+3];
		Xil_Out32(FRANKENCRYPT_DATA_REGISTER, word);
	}
	for (int i = 0; i < RSA_KEY_SZ; i+=4)
	{
		uint32_t word = (n[i]<<24) | (n[i+1]<<16) | (n[i+2]<<8) | n[i+3];
		Xil_Out32(FRANKENCRYPT_DATA_REGISTER, word);
	}

	//Trigger RSA
	Xil_Out32(FRANKENCRYPT_COMMAND_REGISTER, TRIGGER_RSA );
	//Reset Trigger
	Xil_Out32(FRANKENCRYPT_COMMAND_REGISTER, TRIGGER_RSA );
}



void rsa_get_verify_out(uint8_t * out)
{
	//Wait for RSA to finish.
	while( !rsa_complete() ){}

	//Read out computed stuff
	Xil_Out32(FRANKENCRYPT_ADDRESS_REGISTER, FRANKENCRYPT_ADDRESS_RSA_DATA_OUT); //Setup Address
	for (int i = 0; i < 128; i+=4)
	{
		u32 out_word = Xil_In32(FRANKENCRYPT_DATA_REGISTER);
		out[i] = out_word >> 8*3;
		out[i+1] = out_word >> 8*2;
		out[i+2] = out_word >> 8*1;
		out[i+3] = out_word;
	}
}




