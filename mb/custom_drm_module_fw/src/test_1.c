
#ifdef TEST1

int main(void)
{
	u32 status;
    init_platform();
	microblaze_register_handler((XInterruptHandler)myISR, (void *)0);
	microblaze_enable_interrupts();

	// Initialize the interrupt controller driver so that it is ready to use.
	status = XIntc_Initialize(&InterruptController, XPAR_INTC_0_DEVICE_ID);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Set up the Interrupt System.
	status = SetUpInterruptSystem(&InterruptController, (XInterruptHandler)myISR);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	u8 text1[] = {"abc"};
	u8 text2[] = {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"};
	u8 text3[] = {"aaaaaaaaaa"};
	u8 hash1[SHA256_BLOCK_SIZE] = {0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,
									 0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad};
	u8 hash2[SHA256_BLOCK_SIZE] = {0x24,0x8d,0x6a,0x61,0xd2,0x06,0x38,0xb8,0xe5,0xc0,0x26,0x93,0x0c,0x3e,0x60,0x39,
									 0xa3,0x3c,0xe4,0x59,0x64,0xff,0x21,0x67,0xf6,0xec,0xed,0xd4,0x19,0xdb,0x06,0xc1};
	u8 hash3[SHA256_BLOCK_SIZE] = {0xcd,0xc7,0x6e,0x5c,0x99,0x14,0xfb,0x92,0x81,0xa1,0xc7,0xe2,0x84,0xd7,0x3e,0x67,
									 0xf1,0x80,0x9a,0x48,0xa4,0x97,0x20,0x0e,0x04,0x6d,0x39,0xcc,0xc7,0x11,0x2c,0xd0};
	u8 buf[SHA256_BLOCK_SIZE];

	sha256_compute_hash((void*)text1, 3, 1, buf);

    if (compare_hashes((char*) buf, (char*) hash1)) {
    	printf("Test Pass \n\r");
    } else {
    	printf("Test Fail \n\r");
//		for( int i = 0; i < 16; i++){
//			printf("%02x \r\n", buf[i]);
//		}
    }
    while(1){

    }
    cleanup_platform();
    return 0;
}
/*
int main(void)
{
	u32 status;
    init_platform();
	microblaze_register_handler((XInterruptHandler)myISR, (void *)0);
	microblaze_enable_interrupts();

	// Initialize the interrupt controller driver so that it is ready to use.
	status = XIntc_Initialize(&InterruptController, XPAR_INTC_0_DEVICE_ID);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Set up the Interrupt System.
	status = SetUpInterruptSystem(&InterruptController, (XInterruptHandler)myISR);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	u8 text1[] = {"dddddlkasdfjjjjjjjjjjjjjjjjjjjjjjjjjjjjjasdfasdfapoiupoiuspdofuaposdifpaoisdufpaoisdufpoaisudpfoiuaspdoifuapsoidufpaoisdufpoaisdufpaoisudfpoaisudfpoaisudpfoi"};
	u8 hash1[SHA256_BLOCK_SIZE] = {0x56, 0xab, 0xc4, 0x82, 0xfd, 0x05, 0x97, 0xdb, 0x12, 0xae, 0xcd, 0x13, 0xa8, 0x76, 0x80, 0x73, 0x3a, 0x4a, 0x59, 0xdb, 0xf6, 0x62, 0x1e, 0x58, 0x73, 0xa0, 0x62, 0x56, 0x05, 0x8b, 0x18, 0x7b};
	u8 buf[SHA256_BLOCK_SIZE];

	sha256_compute_hash((void*)text1, sizeof(text1)-1, 1, buf);

    if (compare_hashes((char*) buf, (char*) hash1)) {
    	printf("Test Pass \n\r");
    } else {
    	printf("Test Fail \n\r");
		for( int i = 0; i < 32; i++){
			printf("%02x \r\n", buf[i]);
		}
    }
    while(1){

    }
    cleanup_platform();
    return 0;
}*/


/*
int main(void)
{
	u32 status;
    init_platform();
	microblaze_register_handler((XInterruptHandler)myISR, (void *)0);
	microblaze_enable_interrupts();

	// Initialize the interrupt controller driver so that it is ready to use.
	status = XIntc_Initialize(&InterruptController, XPAR_INTC_0_DEVICE_ID);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Set up the Interrupt System.
	status = SetUpInterruptSystem(&InterruptController, (XInterruptHandler)myISR);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	u8 text1[] = {"dddddlkasdfjjjjjjjjjjjjjjjjjjjjjjjjjjjjjasdfasdfapoiupoiuspdofuaposdifpaoisdufpaoisdufpoaisudpfoiuaspdoifuapsoidufpaoisdufpoaisdufpaoisudfpoaisudfpoaisudpfoi"};
	u8 hash1[SHA256_BLOCK_SIZE] = {0x56, 0xab, 0xc4, 0x82, 0xfd, 0x05, 0x97, 0xdb, 0x12, 0xae, 0xcd, 0x13, 0xa8, 0x76, 0x80, 0x73, 0x3a, 0x4a, 0x59, 0xdb, 0xf6, 0x62, 0x1e, 0x58, 0x73, 0xa0, 0x62, 0x56, 0x05, 0x8b, 0x18, 0x7b};
	u8 buf[SHA256_BLOCK_SIZE];

	sha256_compute_hash((void*)text1, sizeof(text1)-1, 1, buf);



    if (compare_hashes((char*) buf, (char*) hash1)) {
    	printf("Test Pass \n\r");
    } else {
    	printf("Test Fail \n\r");
		for( int i = 0; i < 32; i++){
			printf("%02x \r\n", buf[i]);
		}
    }

	sha256_compute_hash((void*)text1, sizeof(text1)-1, 1, buf);



    if (compare_hashes((char*) buf, (char*) hash1)) {
    	printf("Test Pass \n\r");
    } else {
    	printf("Test Fail \n\r");
		for( int i = 0; i < 32; i++){
			printf("%02x \r\n", buf[i]);
		}
    }

    u8 text2[] = {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"};
    u8 hash2[SHA256_BLOCK_SIZE] = {0x24,0x8d,0x6a,0x61,0xd2,0x06,0x38,0xb8,0xe5,0xc0,0x26,0x93,0x0c,0x3e,0x60,0x39,
    									 0xa3,0x3c,0xe4,0x59,0x64,0xff,0x21,0x67,0xf6,0xec,0xed,0xd4,0x19,0xdb,0x06,0xc1};

    sha256_compute_hash((void*)text2, sizeof(text2)-1, 1, buf);
    if (compare_hashes((char*) buf, (char*) hash2)) {
        	printf("Test Pass \n\r");
        } else {
        	printf("Test Fail \n\r");
    		for( int i = 0; i < 32; i++){
    			printf("%02x \r\n", buf[i]);
    		}
        }


    while(1){

    }
    cleanup_platform();
    return 0;
}*/




/*
 *
 *  *****************************  ADD SUBTRACT TEST **************************************
void randomize(uint8_t * arr, int len)
{
	u32 random;

	for( int i = 0; i < len; i++){
		random=rand()% 0x100; //make uint8
		arr[i] = (uint8_t) random;
	}
}

void add(uint8_t * A, uint8_t * B, uint8_t * out)
{
	u16 carry = 0;
	for( int i = 127; i >= 0; i--){
		u16 a = A[i];
		u16 b = B[i];
		carry = carry + a + b;
		out[i] = carry % 0x100;
		carry >>= 8;
		carry = carry % 2;
	}
}

void sub(uint8_t * A, uint8_t * B, uint8_t * out)
{
	u16 carry = 1;
	for( int i = 127; i >= 0; i--){
		u16 a = A[i];
		u16 b = B[i];
		carry = a + ~b + carry;
		out[i] = carry % 0x100;
		carry >>= 8;
		carry = (carry % 2) ^ 1;
	}
}


int main(void)
{
	u32 status;
    init_platform();
	microblaze_register_handler((XInterruptHandler)myISR, (void *)0);
	microblaze_enable_interrupts();

	// Initialize the interrupt controller driver so that it is ready to use.
	status = XIntc_Initialize(&InterruptController, XPAR_INTC_0_DEVICE_ID);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Set up the Interrupt System.
	status = SetUpInterruptSystem(&InterruptController, (XInterruptHandler)myISR);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//enableLED(led);
	printf("Starting Adder test \r\n");

	int test_vector_ct = 10;

	uint8_t out[128] = {0};

	uint8_t t_out[128] = {0};

	uint8_t A[128] = {0};

	uint8_t B[128] = {0};

	uint8_t dud[128] = {0};
	srand(55);

	for (int i = 0; i < test_vector_ct; i++){
		randomize(A, 128);

		randomize(B, 128);

		add(A,B, t_out);
		dud[0] = 2;
		rsa_encrypt( A, B, dud,  out);

	    if (0 == memcmp((char*) out, (char*) t_out, 128)) {
	    	set_playing();
	    	printf("Test Passed Add %d\n\r", i);

	    } else {
	    	set_paused();
	    	printf("Test Failed Add %d\n\r", i);
			printf("A B t_out out \n\r");
			for (int i = 0; i < 128; i++)
			{
				printf("%02x %02x %02x %02x\n\r", A[i], B[i], t_out[i], out[i]);
			}
	    }

	    sub(A,B, t_out);
	    dud[0] = 0;
		rsa_encrypt( A, B, dud,  out);

	    if (0 == memcmp((char*) out, (char*) t_out, 128)) {
	    	set_playing();
	    	printf("Test Passed Sub %d\n\r", i);
//	    	printf("A B t_out out \n\r");
//			for (int i = 0; i < 128; i++)
//			{
//				printf("%02x %02x %02x %02x\n\r", A[i], B[i], t_out[i], out[i]);
//			}
	    } else {
	    	set_paused();
	    	printf("Test Failed Sub %d\n\r", i);
			printf("A B t_out out \n\r");
			for (int i = 0; i < 128; i++)
			{
				printf("%02x %02x %02x %02x\n\r", A[i], B[i], t_out[i], out[i]);
			}
	    }
	}




    while(1){

    }
    cleanup_platform();
    return 0;
}
*  ***************************** END ADD SUBTRACT TEST **************************************
*/

/*
int main(void)
{
	u32 status;
    init_platform();
	microblaze_register_handler((XInterruptHandler)myISR, (void *)0);
	microblaze_enable_interrupts();

	// Initialize the interrupt controller driver so that it is ready to use.
	status = XIntc_Initialize(&InterruptController, XPAR_INTC_0_DEVICE_ID);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Set up the Interrupt System.
	status = SetUpInterruptSystem(&InterruptController, (XInterruptHandler)myISR);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//enableLED(led);

    uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
    uint8_t in[]  = { 0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46, 0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d,
                      0x50, 0x86, 0xcb, 0x9b, 0x50, 0x72, 0x19, 0xee, 0x95, 0xdb, 0x11, 0x3a, 0x91, 0x76, 0x78, 0xb2,
                      0x73, 0xbe, 0xd6, 0xb8, 0xe3, 0xc1, 0x74, 0x3b, 0x71, 0x16, 0xe6, 0x9e, 0x22, 0x22, 0x95, 0x16,
                      0x3f, 0xf1, 0xca, 0xa1, 0x68, 0x1f, 0xac, 0x09, 0x12, 0x0e, 0xca, 0x30, 0x75, 0x86, 0xe1, 0xa7 };

    uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    uint8_t out[] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
                      0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
                      0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
                      0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10 };
    //uint8_t out_buf[64] = { 0 };

    struct AES_ctx ctx;

    AES_init_ctx_iv(&ctx, key, iv);

    //AES_CBC_decrypt_copy(&ctx, in, out_buf, 64);
    AES_CBC_decrypt_buffer(&ctx, in, 64);

    for( int i = 0; i < 64; i++){
		printf("%02x \r\n", in[i]);
	}

    //printf("Boop %d \n", in[3]);

    if (0 == memcmp((char*) out, (char*) in, 64)) {
    	set_playing();
    	printf("Test Passed \n\r");

    } else {
    	set_paused();
    	printf("Test Failed \n\r");
    }
    while(1){

    }
    cleanup_platform();
    return 0;
}*/


/*
int main(void)
{
	u32 status;
    init_platform();
	microblaze_register_handler((XInterruptHandler)myISR, (void *)0);
	microblaze_enable_interrupts();

	// Initialize the interrupt controller driver so that it is ready to use.
	status = XIntc_Initialize(&InterruptController, XPAR_INTC_0_DEVICE_ID);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Set up the Interrupt System.
	status = SetUpInterruptSystem(&InterruptController, (XInterruptHandler)myISR);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	u8 text1[] = {"dddddlkasdfjjjjjjjjjjjjjjjjjjjjjjjjjjjjjasdfasdfapoiupoiuspdofuaposdifpaoisdufpaoisdufpoaisudpfoiuaspdoifuapsoidufpaoisdufpoaisdufpaoisudfpoaisudfpoaisudpfoi"};
	u8 hash1[SHA256_BLOCK_SIZE] = {0x56, 0xab, 0xc4, 0x82, 0xfd, 0x05, 0x97, 0xdb, 0x12, 0xae, 0xcd, 0x13, 0xa8, 0x76, 0x80, 0x73, 0x3a, 0x4a, 0x59, 0xdb, 0xf6, 0x62, 0x1e, 0x58, 0x73, 0xa0, 0x62, 0x56, 0x05, 0x8b, 0x18, 0x7b};
	u8 buf[SHA256_BLOCK_SIZE];

	sha256_compute_hash((void*)text1, sizeof(text1)-1, 1, buf);



    if (compare_hashes((char*) buf, (char*) hash1)) {
    	printf("Test Pass \n\r");
    } else {
    	printf("Test Fail \n\r");
		for( int i = 0; i < 32; i++){
			printf("%02x \r\n", buf[i]);
		}
    }

	sha256_compute_hash((void*)text1, sizeof(text1)-1, 1, buf);



    if (compare_hashes((char*) buf, (char*) hash1)) {
    	printf("Test Pass \n\r");
    } else {
    	printf("Test Fail \n\r");
		for( int i = 0; i < 32; i++){
			printf("%02x \r\n", buf[i]);
		}
    }

    u8 text2[] = {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"};
    u8 hash2[SHA256_BLOCK_SIZE] = {0x24,0x8d,0x6a,0x61,0xd2,0x06,0x38,0xb8,0xe5,0xc0,0x26,0x93,0x0c,0x3e,0x60,0x39,
    									 0xa3,0x3c,0xe4,0x59,0x64,0xff,0x21,0x67,0xf6,0xec,0xed,0xd4,0x19,0xdb,0x06,0xc1};

    sha256_compute_hash((void*)text2, sizeof(text2)-1, 1, buf);
    if (compare_hashes((char*) buf, (char*) hash2)) {
        	printf("Test Pass \n\r");
        } else {
        	printf("Test Fail \n\r");
    		for( int i = 0; i < 32; i++){
    			printf("%02x \r\n", buf[i]);
    		}
        }


    while(1){

    }
    cleanup_platform();
    return 0;
}
*/


/*
int main(void)
{
	u32 status;
    init_platform();
	microblaze_register_handler((XInterruptHandler)myISR, (void *)0);
	microblaze_enable_interrupts();

	// Initialize the interrupt controller driver so that it is ready to use.
	status = XIntc_Initialize(&InterruptController, XPAR_INTC_0_DEVICE_ID);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Set up the Interrupt System.
	status = SetUpInterruptSystem(&InterruptController, (XInterruptHandler)myISR);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	u8 text1[] = {"abc"};
	u8 text2[] = {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"};
	u8 text3[] = {"aaaaaaaaaa"};
	u8 hash1[SHA256_BLOCK_SIZE] = {0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,
									 0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad};
	u8 hash2[SHA256_BLOCK_SIZE] = {0x24,0x8d,0x6a,0x61,0xd2,0x06,0x38,0xb8,0xe5,0xc0,0x26,0x93,0x0c,0x3e,0x60,0x39,
									 0xa3,0x3c,0xe4,0x59,0x64,0xff,0x21,0x67,0xf6,0xec,0xed,0xd4,0x19,0xdb,0x06,0xc1};
	u8 hash3[SHA256_BLOCK_SIZE] = {0xcd,0xc7,0x6e,0x5c,0x99,0x14,0xfb,0x92,0x81,0xa1,0xc7,0xe2,0x84,0xd7,0x3e,0x67,
									 0xf1,0x80,0x9a,0x48,0xa4,0x97,0x20,0x0e,0x04,0x6d,0x39,0xcc,0xc7,0x11,0x2c,0xd0};
	u8 buf[SHA256_BLOCK_SIZE];

	sha256_compute_hash((void*)text2, 52 , 1, buf);

    if (compare_hashes((char*) buf, (char*) hash2)) {
    	printf("Test Pass \n\r");
    } else {
    	printf("Test Fail \n\r");
//		for( int i = 0; i < 16; i++){
//			printf("%02x \r\n", buf[i]);
//		}
    }
    while(1){

    }
    cleanup_platform();
    return 0;
}
*/




/*
int main(void)
{
	u32 status;
    init_platform();
	microblaze_register_handler((XInterruptHandler)myISR, (void *)0);
	microblaze_enable_interrupts();

	// Initialize the interrupt controller driver so that it is ready to use.
	status = XIntc_Initialize(&InterruptController, XPAR_INTC_0_DEVICE_ID);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Set up the Interrupt System.
	status = SetUpInterruptSystem(&InterruptController, (XInterruptHandler)myISR);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	u8 text1[] = {"dddddlkasdfjjjjjjjjjjjjjjjjjjjjjjjjjjjjjasdfasdfapoiupoiuspdofuaposdifpaoisdufpaoisdufpoaisudpfoiuaspdoifuapsoidufpaoisdufpoaisdufpaoisudfpoaisudfpoaisudpfoi"};
	u8 hash1[SHA256_BLOCK_SIZE] = {0x56, 0xab, 0xc4, 0x82, 0xfd, 0x05, 0x97, 0xdb, 0x12, 0xae, 0xcd, 0x13, 0xa8, 0x76, 0x80, 0x73, 0x3a, 0x4a, 0x59, 0xdb, 0xf6, 0x62, 0x1e, 0x58, 0x73, 0xa0, 0x62, 0x56, 0x05, 0x8b, 0x18, 0x7b};
	u8 buf[SHA256_BLOCK_SIZE];

	sha256_compute_hash((void*)text1, sizeof(text1)-1, 1, buf);



    if (compare_hashes((char*) buf, (char*) hash1)) {
    	printf("Test Pass \n\r");
    } else {
    	printf("Test Fail \n\r");
		for( int i = 0; i < 32; i++){
			printf("%02x \r\n", buf[i]);
		}
    }

	sha256_compute_hash((void*)text1, sizeof(text1)-1, 1, buf);



    if (compare_hashes((char*) buf, (char*) hash1)) {
    	printf("Test Pass \n\r");
    } else {
    	printf("Test Fail \n\r");
		for( int i = 0; i < 32; i++){
			printf("%02x \r\n", buf[i]);
		}
    }

    u8 text2[] = {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"};
    u8 hash2[SHA256_BLOCK_SIZE] = {0x24,0x8d,0x6a,0x61,0xd2,0x06,0x38,0xb8,0xe5,0xc0,0x26,0x93,0x0c,0x3e,0x60,0x39,
    									 0xa3,0x3c,0xe4,0x59,0x64,0xff,0x21,0x67,0xf6,0xec,0xed,0xd4,0x19,0xdb,0x06,0xc1};

    sha256_compute_hash((void*)text2, sizeof(text2)-1, 1, buf);
    if (compare_hashes((char*) buf, (char*) hash2)) {
        	printf("Test Pass \n\r");
        } else {
        	printf("Test Fail \n\r");
    		for( int i = 0; i < 32; i++){
    			printf("%02x \r\n", buf[i]);
    		}
        }


    while(1){

    }
    cleanup_platform();
    return 0;
}*/



#endif
