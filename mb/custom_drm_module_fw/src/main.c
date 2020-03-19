/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xstatus.h"
#include "xaxidma.h"
#include "xil_mem.h"
#include "util.h"
#include "secrets.h"
#include "xintc.h"
#include "constants.h"
#include "sleep.h"

#include "HASH/sha256.h"
#include "AES/aes.h"
#include "RSA/rsa.h"

//For testing random
#include <stdlib.h>

//////////////////////// GLOBALS ////////////////////////


// audio DMA access
static XAxiDma sAxiDma;

// shared buffer values
enum commands { QUERY_PLAYER, QUERY_SONG, LOGIN, LOGOUT, SHARE, PLAY, STOP, DIGITAL_OUT, PAUSE, RESTART, FF, RW };
enum states   { STOPPED, WORKING, PLAYING, PAUSED };

// change states
#define change_state(state) c->drm_state = state;
#define set_stopped() change_state(STOPPED)
#define set_working() change_state(WORKING)
#define set_playing() change_state(PLAYING)
#define set_paused()  change_state(PAUSED)

// shared command channel -- read/write for both PS and PL
volatile cmd_channel *c = (cmd_channel*)SHARED_DDR_BASE;

static const uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

// internal state store
internal_state s;

static u8 hash_buffer[SHA256_HASH_SZ]; //Used for storing the output of SHA256

static u8 user_key_block[HASH_SZ]; //Holds the user key while they are logged in

static u8 user_rsa_private_key_block[RSA_KEY_SZ]; //Holds the user private key while they are logged in

static u8 region_key_block[HASH_SZ]; //Holds common key after is_locked() is called

static u8 universal_buffer[sizeof(song_chunk)];

static u8 rsa_output_buffer[RSA_KEY_SZ]; //Used to hold the output of RSA operations

//////////////////////// INTERRUPT HANDLING ////////////////////////


// shared variable between main thread and interrupt processing thread
volatile static int InterruptProcessed = FALSE;
static XIntc InterruptController;

void myISR(void) {
    InterruptProcessed = TRUE;
}

//////////////////////// HELPER FUNCTIONS  ////////////////////////

//returns true if they are the same
int compare_hashes(u8 * hash1, u8 * hash2)
{
	int ret = 1;
	for( int i = 0; i < HASH_SZ; i++)
	{
		if( hash1[i] != hash2[i])
		{
			ret = 0;
		}
	}
	return ret;
}

int compare_verification_hashes(u8 * hash1, u8 * hash2)
{
	int ret = 1;
	for( int i = 0; i < VERIFY_HASH_SZ; i++)
	{
		if( hash1[i] != hash2[i])
		{
			ret = 0;
		}
	}
	return ret;
}




//////////////////////// COMMAND FUNCTIONS ////////////////////////


// attempt to log in to the credentials in the shared buffer
void login() {
    if (s.logged_in) {
        mb_printf("Already logged in. Please log out first.\r\n");
    } else {
    	memcpy(s.username, (void*)c->username, USERNAME_SZ);
		memcpy(s.pin, (void*)c->pin, MAX_PIN_SZ);

		// Check string lengths
		size_t pin_length = strlen(s.pin);
		size_t username_length = strlen(s.username);
		if ((pin_length > MAX_PIN_SZ) || (username_length > USERNAME_SZ)){
			// reject login attempt
			mb_printf("Invalid input\r\n");
			memset((void*)c->username, 0, USERNAME_SZ);
			memset((void*)c->pin, 0, MAX_PIN_SZ);
			return;
		}

		struct AES_ctx ctx;

		//Start HASH computation
    	sha256_compute_hash((void*)s.pin, pin_length, 1, hash_buffer);

    	AES_init_ctx(&ctx, hash_buffer);


        for (int i = 0; i < NUM_PROVISIONED_USERS; i++) {
            // search for matching username
            if (!strcmp((void*)s.username, USERNAMES[PROVISIONED_UIDS[i]])) {
                // check if pin matches
            	memcpy(user_key_block, PROVISIONED_USER_KEY_BLOCKS[i], HASH_SZ);

            	AES_ECB_decrypt(&ctx, user_key_block); //Decrypt the block. Should now be equal to device hash

            	if ( compare_hashes((void*)user_key_block, (void*)DEVICE_HASH) ){
            		//update states
            		s.logged_in = 1;
                    c->login_status = 1;
                    s.uid = PROVISIONED_UIDS[i];
                    mb_printf("Logged in for user '%s'\r\n", c->username);
                    memcpy(user_key_block, hash_buffer, HASH_SZ); // Copy user key into user_key_block

                    //Decrypt private key
                    memcpy(user_rsa_private_key_block, PROVISIONED_USER_PRIVATE_KEY_D_BLOCKS[s.uid], RSA_KEY_SZ);
                    AES_init_ctx_iv(&ctx, user_key_block, iv);
                    AES_CBC_decrypt_buffer(&ctx, user_rsa_private_key_block, RSA_KEY_SZ);
                    return;
            	} else {
                    // reject login attempt
                    mb_printf("Incorrect pin for user '%s'\r\n", c->username);
                    memset((void*)c->username, 0, USERNAME_SZ);
                    memset((void*)c->pin, 0, MAX_PIN_SZ);
                    memset((void*)s.username, 0, USERNAME_SZ);
					memset((void*)s.pin, 0, MAX_PIN_SZ);
                    return;
                }
            }
        }

        // reject login attempt
        mb_printf("User not found\r\n");
        memset((void*)c->username, 0, USERNAME_SZ);
        memset((void*)c->pin, 0, MAX_PIN_SZ);
    }
}

// attempt to log out
void logout() {
    if (s.logged_in) {
        mb_printf("Logging out...\r\n");
        s.logged_in = 0;
        c->login_status = 0;
        memset((void*)c->username, 0, USERNAME_SZ);
        memset((void*)c->pin, 0, MAX_PIN_SZ);
        memset((void*)user_key_block, 0,HASH_SZ); //Delete user key
        memset((void*)user_rsa_private_key_block, 0,  RSA_KEY_SZ); //Delete use private key
        memset((void*)hash_buffer, 0, SHA256_HASH_SZ);
        s.uid = 0;
    } else {
        mb_printf("Not logged in\r\n");
    }
}

void query_player() {
    c->query.num_regions = NUM_PROVISIONED_REGIONS;
    c->query.num_users = NUM_PROVISIONED_USERS;

    for (int i = 0; i < NUM_PROVISIONED_REGIONS; i++) {
        strcpy((char *)q_region_lookup(c->query, i), REGION_NAMES[PROVISIONED_RIDS[i]]);
    }

    for (int i = 0; i < NUM_PROVISIONED_USERS; i++) {
        strcpy((char *)q_user_lookup(c->query, i), USERNAMES[i]);
    }

    mb_printf("Queried player (%d regions, %d users)\r\n", c->query.num_regions, c->query.num_users);
}

// loads the song metadata in the shared buffer into the local struct
void load_song_md() {
	memcpy((void *)&s.song_md, (void *)&c->song.song_header, sizeof(header));
	memcpy((void *)&s.song_shared_user_md, (void *)&c->song.shared_user_block, sizeof(shared_users));
}

int verify_song_md_hash()
{
	sha256_compute_hash((void*)&s.song_md, sizeof(header)-HASH_SZ-RSA_KEY_SZ, 1, hash_buffer);
	return compare_hashes(hash_buffer, s.song_md.header_hash);
}

// rsa_begin_verify must be called before using this
int verify_song_hash_signature( uint8_t * hash)//, uint8_t * signature , uint8_t * key, uint8_t * n)
{
	uint8_t * expected = (void*)hash;

	rsa_get_verify_out(rsa_output_buffer);

	int ret1 = 1;

	for(int i = 0; i < RSA_KEY_SZ - HASH_SZ ; i++){
		if(rsa_output_buffer[i] != 0){
			ret1 = 0;
			break;
		}
	}
	for(int i = 0; i < HASH_SZ ; i++){
		if(rsa_output_buffer[i + (RSA_KEY_SZ - HASH_SZ)] != expected[i]){
			ret1 = 0;
			break;
		}
	}

	return ret1;
}


int verify_song_md_hash_signature()
{
	uint8_t * expected = (void*)s.song_md.header_hash;
	rsa_begin_verify( (void*)s.song_md.hash_signature, (void*)GLOBAL_PUBLIC_E, (void*)GLOBAL_PUBLIC_N);
	return verify_song_hash_signature( expected );
}

// load_song_md() should be called first
// if the song is region enabled this function decrypts the common_key and loads it into region_key_block[]
int is_region_locked(){
	int region_locked = TRUE; // reset lock for region check
	int enabled_region_bits = s.song_md.enabled_regions.regions;
	for (int j = 0; j < (u8)NUM_PROVISIONED_REGIONS; j++) {
		if ((enabled_region_bits >> PROVISIONED_RIDS[j]) & 1 ){
			//Load region key block from song into region_key_block buffer
			int region_key_offset = HASH_SZ * PROVISIONED_RIDS[j];
			memcpy((void*)region_key_block, (s.song_md.enabled_regions.region_keys + region_key_offset) , HASH_SZ );
			//Decrypt region_key_block using region key from secrets.h

			struct AES_ctx ctx;
			AES_init_ctx(&ctx, REGION_KEYS[PROVISIONED_RIDS[j]]); //init AES key
			AES_ECB_decrypt(&ctx, region_key_block); //Decrypt the block. Should now be equal to common_key

			region_locked = FALSE;
			break;
		}
	}
	return region_locked;
}

// checks if the song loaded into the shared buffer is locked for the current user
// load_song_md() should be called first
// if the song is region enabled this function decrypts the common_key and loads it into region_key_block[]
int is_locked(uint8_t * is_shared_out) {
    int user_locked = TRUE;
    int region_locked = TRUE;

    *is_shared_out = 0; //Default to not shared

    // check for authorized user
    if (!s.logged_in) {
        mb_printf("No user logged in");
    } else {
        //load_song_md();

        // check if user is authorized to play song
        if (s.uid == s.song_md.owner_id) {
        	user_locked = FALSE;
        } else {
        	// Check if the song is shared with current user
        	uint64_t shared_mask = 1 << s.uid;
        	if( (c->song.shared_user_block.enabled_users & shared_mask) > 0){
        		//User may be allowed to play this song
        		user_locked = FALSE;
        		*is_shared_out = 1; //Lets play_song know that this is a shared user
        	}
        }

        if (user_locked) {
            mb_printf("User '%s' does not have access to this song", s.username);
            return user_locked;
        }
        mb_printf("User '%s' has access to this song", s.username);

        region_locked = is_region_locked(); // This loads the common key into the region_key_block

        if (!region_locked) {
            mb_printf("Region Match. Full Song can be played. Unlocking...");
        } else {
            mb_printf("Invalid region");
        }
    }
    return region_locked;
}

// returns whether a uid has been provisioned
int is_provisioned_uid(char uid) {
    for (int i = 0; i < NUM_PROVISIONED_USERS; i++) {
        if (uid == PROVISIONED_UIDS[i]) {
            return TRUE;
        }
    }
    return FALSE;
}

// looks up the username corresponding to the uid
int uid_to_username(char uid, char **username, int provisioned_only) {
    for (int i = 0; i < NUM_USERS; i++) {
        if (uid == USER_IDS[i] &&
            (!provisioned_only || is_provisioned_uid(uid))) {
            *username = (char *)USERNAMES[i];
            return TRUE;
        }
    }
    mb_printf("Could not find uid '%d'\r\n", uid);
    *username = "<unknown user>";
    return FALSE;
}

// looks up the uid corresponding to the username
int username_to_uid(char *username, char *uid, int provisioned_only) {
    for (int i = 0; i < NUM_USERS; i++) {
        if (!strcmp(username, USERNAMES[USER_IDS[i]]) &&
            (!provisioned_only || is_provisioned_uid(USER_IDS[i]))) {
            *uid = USER_IDS[i];
            return TRUE;
        }
    }

    mb_printf("Could not find username '%s'\r\n", username);
    *uid = -1;
    return FALSE;
}

// returns whether an rid has been provisioned
int is_provisioned_rid(char rid) {
    for (int i = 0; i < NUM_PROVISIONED_REGIONS; i++) {
        if (rid == PROVISIONED_RIDS[i]) {
            return TRUE;
        }
    }
    return FALSE;
}

// looks up the region name corresponding to the rid
int rid_to_region_name(char rid, char **region_name, int provisioned_only) {
    for (int i = 0; i < NUM_REGIONS; i++) {
        if (rid == REGION_IDS[i] &&
            (!provisioned_only || is_provisioned_rid(rid))) {
            *region_name = (char *)REGION_NAMES[i];
            return TRUE;
        }
    }

    mb_printf("Could not find region ID '%d'\r\n", rid);
    *region_name = "<unknown region>";
    return FALSE;
}


// looks up the rid corresponding to the region name
int region_name_to_rid(char *region_name, char *rid, int provisioned_only) {
    for (int i = 0; i < NUM_REGIONS; i++) {
        if (!strcmp(region_name, REGION_NAMES[i]) &&
            (!provisioned_only || is_provisioned_rid(REGION_IDS[i]))) {
            *rid = REGION_IDS[i];
            return TRUE;
        }
    }

    mb_printf("Could not find region name '%s'\r\n", region_name);
    *rid = -1;
    return FALSE;
}

// handles a request to query song metadata
void query_song() {
    char *name;

    // load song
    load_song_md();

    //TODO For some reason the first time that query is called verify song md hash fails. This causes miPod to pring gibberish
//    if( !verify_song_md_hash()){
//    	mb_printf("Unrecognized Song File /n/r");
//    	return;
//    }
//
//    if( !verify_song_md_hash_signature()){
//    	mb_printf("Song File could not be verified /n/r");
//    	return;
//    }
    memset((void *)&c->query, 0, sizeof(query));

    int enabled_region_bits = s.song_md.enabled_regions.regions;
    int region_count = 0;
    for( int i = 0; i < 32; i++)
    {
    	if (enabled_region_bits & 1){
    		rid_to_region_name(i, &name, FALSE);
    		strcpy((char *)q_region_lookup(c->query, region_count), name);
    		region_count += 1;
    	}
    	enabled_region_bits >>= 1;
    }

    c->query.num_regions = region_count;


    // copy owner name
    uid_to_username(s.song_md.owner_id, &name, FALSE);
    strcpy((char *)c->query.owner, name);


    uint64_t enabled_user_bits = s.song_shared_user_md.enabled_users;
    mb_printf("enabled users %d \n\r", enabled_user_bits);
    int user_count = 0;
	for( int i = 0; i < 63; i++)
	{
		if (enabled_user_bits & 1){
			uid_to_username(i, &name, FALSE);
			strcpy((char *)q_user_lookup(c->query, user_count ), name);
			user_count += 1;
		}
		enabled_user_bits >>= 1;
	}
	c->query.num_users = user_count; //TODO Test implement sharing

   mb_printf("Queried song (%d regions, %d users)\r\n", c->query.num_regions, c->query.num_users);
}

void play_song() {
    u32 counter = 0, rem, cp_num, cp_xfil_cnt, offset, dma_cnt, length, *fifo_fill;


    mb_printf("Reading Audio File...");
    load_song_md();


    sha256_compute_hash((void*)&s.song_md, sizeof(header)-HASH_SZ-RSA_KEY_SZ, 1, hash_buffer);
    if( !compare_hashes(hash_buffer, s.song_md.header_hash)){
    	mb_printf("Unrecognized Song File \n\r");
    	return;
    }

    if( !verify_song_md_hash_signature()){ //This operation uses the Universal Buffer
    	mb_printf("Song File could not be verified \n\r");
    	return;
    }

    // get WAV length
    length = s.song_md.song_size;

    //Verify song_verify_hash. Raise the flag to high.
    uint8_t verify_song_hash_flag = 1;



    struct AES_ctx ctx;

    // truncate song if locked
    int is_truncated = 1;
    uint8_t is_shared_user_flag = 0;
    if (length > PREVIEW_SZ && is_locked(&is_shared_user_flag)) {
        //length = PREVIEW_SZ;
        mb_printf("Song is locked.  Playing only %ds = %dB\r\n",
                   PREVIEW_TIME_SEC, PREVIEW_SZ);
    } else {
        mb_printf("Song is unlocked. Playing full song\r\n");
        //Compute song key by concating user key and song hash. Then hashing. Then concating the output with the common key. And hashing again.
        if( is_shared_user_flag == 1){
        	//If the song is being played by a shared user, decrypt the concat song key
        	rsa_encrypt((void*)c->song.shared_user_block.user_key_blocks[s.uid], (void*)user_rsa_private_key_block, (void*)PROVISIONED_USER_PUBLIC_KEY_N_BLOCKS[(uint8_t)s.uid],  rsa_output_buffer);
        	memcpy(hash_buffer, rsa_output_buffer+RSA_KEY_SZ-HASH_SZ, HASH_SZ); // Copy the decrypted result into the hash_buffer
        }else{
        	//If the song is being played by the owner, generate the concat song key
			memcpy(universal_buffer, user_key_block, HASH_SZ);
			memcpy(universal_buffer+HASH_SZ, s.song_md.song_hash, HASH_SZ);
			sha256_compute_hash((void*)universal_buffer , HASH_SZ*2 , 1, hash_buffer);
        }
        memcpy(universal_buffer, hash_buffer, HASH_SZ); // Copy concat song key into universal buffer
        memcpy(universal_buffer+HASH_SZ, region_key_block, HASH_SZ); // append the common key to the end of the concat key
        sha256_compute_hash((void*)universal_buffer , HASH_SZ*2 , 1, hash_buffer); // The first 16 bytes are the actual song key... Finally
        AES_init_ctx_iv(&ctx, hash_buffer, iv); //Initialize AES_CBC key
        is_truncated = 0;
    }

    song_chunk * chunk_pointer; // = (u8*)(&c->song.block_array);
    song_chunk * current_chunk = (void*)universal_buffer;

    rem = length;
    fifo_fill = (u32 *)XPAR_FIFO_COUNT_AXI_GPIO_0_BASEADDR;

    int chunk_ct = 0;
    int number_of_chunks = length%CHUNK_SZ==0 ? length/CHUNK_SZ : length/CHUNK_SZ + 1;
    int last_chunk_len = length%CHUNK_SZ; //The amount of samples in the last chunk. So we don't play junk at the end of the song

    if ( is_truncated){ //Check last chunk first
        chunk_ct = PREVIEW_CHUNK_CT - 1;
    }else{
        chunk_ct = number_of_chunks - 1; 
    }
    
    uint8_t check_last_chunk_flag = 1;
    // write entire file to two-block codec fifo
    // writes to one block while the other is being played
    set_playing();
    while(number_of_chunks > chunk_ct) {
        // check for interrupt to stop playback
        while (InterruptProcessed) {
            InterruptProcessed = FALSE;

            switch (c->cmd) {
            case PAUSE:
                mb_printf("Pausing... \r\n");
                set_paused();
                while (!InterruptProcessed) continue; // wait for interrupt
                break;
            case PLAY:
                mb_printf("Resuming... \r\n");
                set_playing();
                break;
            case STOP:
                mb_printf("Stopping playback...");
                return;
            case FF:
            	mb_printf("Fast Forward... \r\n");
            	chunk_ct += 30;
            	continue;
            case RW:
            	mb_printf("Rewind... \r\n");
            	chunk_ct -= 30;
				continue;
            case RESTART:
                mb_printf("Restarting song... \r\n");
                chunk_ct = 0; // reset song counter
                set_playing();
            default:
                break;
            }
        }

        // calculate write size and offset
        cp_num = (rem > CHUNK_SZ) ? CHUNK_SZ : rem;
        offset = (counter++ % 2 == 0) ? 0 : CHUNK_SZ;


        chunk_pointer = (song_chunk*)(&c->song.block_array[chunk_ct]); 
        //Load chunk into universal_buffer
        memcpy((void*)current_chunk, (void *)(chunk_pointer),(u32)(sizeof(song_chunk)));




        if(chunk_ct < PREVIEW_CHUNK_CT){
        	//preview chunks
        	//Verify signature (Begin process)
        	rsa_begin_verify( (void*)current_chunk->chunk_hash_signature, (void*)GLOBAL_PUBLIC_E, (void*)GLOBAL_PUBLIC_N);

        }else{
        	//encrypted chunks
        	//Decrypt the current chunk
        	if(is_truncated){ //Stop playback if song is truncated
        		return;
        	}
        	memcpy(ctx.Iv, iv, HASH_SZ );//reset iv
        	AES_CBC_decrypt_buffer(&ctx, (void *)(current_chunk), (u32)sizeof(song_chunk));
        }

        u8 chunk_hash_buffer[HASH_SZ];
        memcpy((void*)chunk_hash_buffer, (void*)current_chunk->chunk_hash, HASH_SZ); //Save the chunk hash
        memcpy((void*)current_chunk->chunk_hash, (void*)s.song_md.song_hash, HASH_SZ); //Insert the song hash into the chunk hash space
        //Compute HASH
		sha256_compute_hash((void*)current_chunk , sizeof(song_chunk)-RSA_KEY_SZ , 1, hash_buffer); //Leave out signature during hash computation
		//Verify hash
		if( !compare_hashes((void*)chunk_hash_buffer, (void*)hash_buffer) )
		{
			mb_printf("Song data has been corrupted. Stopping \n\r");
			return;
		}
        //Check that chunk_ct matches
    	if(current_chunk->chunk_number != chunk_ct){
    		mb_printf("Error: song chunk out of order. Stopping \n\r");
    		return;
    	}



		if( chunk_ct == number_of_chunks - 1){
			// If this is the last chunk. Only load valid data
			Xil_MemCpy((void *)(XPAR_MB_DMA_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + offset), (void *)(current_chunk->data),(u32)last_chunk_len);
		}else{
	        // do first mem cpy here into DMA BRAM
			Xil_MemCpy((void *)(XPAR_MB_DMA_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + offset), (void *)(current_chunk->data),(u32)sizeof(song_chunk));
		}



        //DO final checks before playing (Check if signature came back valid)
		if(chunk_ct < PREVIEW_CHUNK_CT){
			if( !verify_song_hash_signature( chunk_hash_buffer)){
				print("Chunk signature verification failed \r\n");
				return;
			}
		}

        if( check_last_chunk_flag == 1){
            if(chunk_ct >= PREVIEW_CHUNK_CT) //Check the signature of the last chunk
            {
                rsa_begin_verify( (void*)current_chunk->chunk_hash_signature, (void*)GLOBAL_PUBLIC_E, (void*)GLOBAL_PUBLIC_N);
                if( !verify_song_hash_signature( chunk_hash_buffer)){
				    print("Chunk signature verification failed \r\n");
				    return;
			    }
            }
            check_last_chunk_flag = 0;
            chunk_ct = 0; //Now go to the start of the song
            continue; //Don't play last chunk first
        }

		if( verify_song_hash_flag == 1) //This must only be done once at the beginning of each song
		{
			mb_printf("Verify song integrity \n\r");
			while(c->song_length == 0){} //Wait for Zynq to process hash

			int plus_one = (s.song_md.song_size % CHUNK_SZ) > 0 ? 1:0;
			int chunked_length = (s.song_md.song_size/CHUNK_SZ + plus_one) * sizeof(song_chunk);
			if(!compare_verification_hashes( (void*)s.song_md.song_verify_hash, (void*)c->song_verify_hash) && (c->song_length==chunked_length)){
				mb_printf("Song hash mismatch. Song file is corrupted \n\r");
//				for(int i = 0; i < HASH_SZ; i++){
//					mb_printf("%02x %02x", s.song_md.song_verify_hash[i], c->song_verify_hash[i]);
//				}
				return;
			}
			verify_song_hash_flag = 0;
		}

        cp_xfil_cnt = cp_num;

        while (cp_xfil_cnt > 0) {

            // polling while loop to wait for DMA to be ready
            // DMA must run first for this to yield the proper state
            // rem != length checks for first run
            while (XAxiDma_Busy(&sAxiDma, XAXIDMA_DMA_TO_DEVICE)
                   && rem != length && *fifo_fill < (FIFO_CAP - 32));

            // do DMA
            dma_cnt = (FIFO_CAP - *fifo_fill > cp_xfil_cnt)
                      ? FIFO_CAP - *fifo_fill
                      : cp_xfil_cnt;
            fnAudioPlay(sAxiDma, offset, dma_cnt);
            cp_xfil_cnt -= dma_cnt;
        }

        rem -= cp_num;
        chunk_ct+=1;
    }
}

// add a user to the song's list of users
void share_song() {
    char uid;

    // reject non-owner attempts to share
    load_song_md();

    if (!s.logged_in) {
        mb_printf("No user is logged in. Cannot share song\r\n");
        c->song.song_header.song_size = 0;
        return;
    } else if (is_region_locked()){ // This loads the common key into the region_key_block
        mb_printf("Region locked. Cannot share song\r\n");
        c->song.song_header.song_size = 0;
        return;
    } else if (s.uid != s.song_md.owner_id) {
        mb_printf("User '%s' is not song's owner. Cannot share song\r\n", s.username);
        c->song.song_header.song_size = 0;
        return;
    } else if (!username_to_uid((char *)c->username, &uid, TRUE)) {
        mb_printf("Username not found\r\n");
        c->song.song_header.song_size = 0;
        return;
    }



    // generate new song metadata
    uint64_t shared_mask = 1 << uid;

    mb_printf("Generating song key \n\r");
    //Generate song key
    memcpy(universal_buffer, user_key_block, HASH_SZ);
	memcpy(universal_buffer+HASH_SZ, (void*)c->song.song_header.song_hash, HASH_SZ);
	sha256_compute_hash((void*)universal_buffer , HASH_SZ*2 , 1, hash_buffer); // The first 16 bytes is the concat song key

	//Get new users public key
	uint8_t * new_use_public_key = (void*)PROVISIONED_USER_PUBLIC_KEY_N_BLOCKS[(uint8_t)uid];

    //Encrypt song key using new users public key
	memset(rsa_output_buffer, 0, RSA_KEY_SZ);
	memcpy(rsa_output_buffer+RSA_KEY_SZ-HASH_SZ, hash_buffer, HASH_SZ); //Copy the key into the rsa buffer
	rsa_encrypt(rsa_output_buffer, (void*)GLOBAL_PUBLIC_E, new_use_public_key,  rsa_output_buffer); //Encrypt the key and write it back to the output buffer

	//Write the encrypted song key into the correct shared user key block
	memcpy((void*)c->song.shared_user_block.user_key_blocks[(uint8_t)uid], rsa_output_buffer, RSA_KEY_SZ);

	//Update shared users
	mb_printf("Writing enabled_users %d \n\r", shared_mask);
	c->song.shared_user_block.enabled_users |= shared_mask;
	mb_printf("New enabled_users %d \n\r", c->song.shared_user_block.enabled_users);
	c->song.song_header.song_size = s.song_md.song_size; //Write song size to confirm sharing. Even though it doesn't change

    mb_printf("Shared song with '%s'\r\n", c->username);
}

// removes DRM data from song for digital out
void digital_out() {
    u32 rem = 0, cp_num, length;

	load_song_md();


	sha256_compute_hash((void*)&s.song_md, sizeof(header)-HASH_SZ-RSA_KEY_SZ, 1, hash_buffer);
	if( !compare_hashes(hash_buffer, s.song_md.header_hash)){
		mb_printf("Unrecognized Song File \n\r");
		return;
	}

	if( !verify_song_md_hash_signature()){ //This operation uses the Universal Buffer
		mb_printf("Song File could not be verified \n\r");
		return;
	}

	// get WAV length
	length = s.song_md.song_size;

	//Verify song_verify_hash
	while(c->song_length == 0){} //Wait for Zynq to process hash

	int plus_one = (s.song_md.song_size % CHUNK_SZ) > 0 ? 1:0;
	int chunked_length = (s.song_md.song_size/CHUNK_SZ + plus_one) * sizeof(song_chunk);
	if(!compare_verification_hashes( (void*)s.song_md.song_verify_hash, (void*)c->song_verify_hash) && (c->song_length==chunked_length)){
		mb_printf("Song hash mismatch. Song file is corrupted \n\r");
		return;
	}

	struct AES_ctx ctx;

	// truncate song if locked
	int is_truncated = 1;
	uint8_t is_shared_user_flag = 0;
	if (length > PREVIEW_SZ && is_locked(&is_shared_user_flag)) {
		c->song_length -= s.song_md.song_size - PREVIEW_SZ;
		mb_printf("Song is locked.  Playing only %ds = %dB\r\n", PREVIEW_TIME_SEC, PREVIEW_SZ);
	} else {
		mb_printf("Song is unlocked. Playing full song\r\n");
		//Compute song key by concating user key and song hash. Then hashing. Then concating the output with the common key. And hashing again.
		if( is_shared_user_flag == 1){
			//If the song is being played by a shared user, decrypt the concat song key
			rsa_encrypt((void*)c->song.shared_user_block.user_key_blocks[s.uid], (void*)user_rsa_private_key_block, (void*)PROVISIONED_USER_PUBLIC_KEY_N_BLOCKS[(uint8_t)s.uid],  rsa_output_buffer);
			memcpy(hash_buffer, rsa_output_buffer+RSA_KEY_SZ-HASH_SZ, HASH_SZ); // Copy the decrypted result into the hash_buffer
		}else{
			//If the song is being played by the owner, generate the concat song key
			memcpy(universal_buffer, user_key_block, HASH_SZ);
			memcpy(universal_buffer+HASH_SZ, s.song_md.song_hash, HASH_SZ);
			sha256_compute_hash((void*)universal_buffer , HASH_SZ*2 , 1, hash_buffer);
		}
		memcpy(universal_buffer, hash_buffer, HASH_SZ); // Copy concat song key into universal buffer
		memcpy(universal_buffer+HASH_SZ, region_key_block, HASH_SZ); // append the common key to the end of the concat key
		sha256_compute_hash((void*)universal_buffer , HASH_SZ*2 , 1, hash_buffer); // The first 16 bytes are the actual song key... Finally
		AES_init_ctx_iv(&ctx, hash_buffer, iv); //Initialize AES_CBC key
		c->song_length = s.song_md.song_size;
		is_truncated = 0;
	}
	mb_printf(MB_PROMPT "Dumping song (%dB)...", c->song_length);

	u8 * chunk_pointer = (u8*)(&c->song.block_array);
	song_chunk * current_chunk = (void*)universal_buffer;

	rem = length;

	int chunk_ct = 0;
	int number_of_chunks = length%CHUNK_SZ==0 ? length/CHUNK_SZ : length/CHUNK_SZ + 1;

    uint8_t verify_song_hash_flag = 1; 
    if ( is_truncated){ //Check last chunk first
        chunk_ct = PREVIEW_CHUNK_CT - 1;
    }else{
        chunk_ct = number_of_chunks - 1; 
    }

	// write entire file to two-block codec fifo
	// writes to one block while the other is being played

	while(number_of_chunks > chunk_ct) {


		// calculate write size
		cp_num = (rem > CHUNK_SZ) ? CHUNK_SZ : rem;

        chunk_pointer = (song_chunk*)(&c->song.block_array[chunk_ct]);
		//Load chunk into universal_buffer
		memcpy((void*)current_chunk, (void *)(chunk_pointer),(u32)(sizeof(song_chunk)));



		if(chunk_ct < PREVIEW_CHUNK_CT){
			//preview chunks
			//Verify signature (Begin process)
			rsa_begin_verify( (void*)current_chunk->chunk_hash_signature, (void*)GLOBAL_PUBLIC_E, (void*)GLOBAL_PUBLIC_N);

		}else{
			//encrypted chunks
			//Decrypt the current chunk
			if(is_truncated){ //Stop playback if song is truncated
				return;
			}
			memcpy(ctx.Iv, iv, HASH_SZ );//reset iv
			AES_CBC_decrypt_buffer(&ctx, (void *)(current_chunk), (u32)sizeof(song_chunk));
		}

		u8 chunk_hash_buffer[HASH_SZ];
		memcpy((void*)chunk_hash_buffer, (void*)current_chunk->chunk_hash, HASH_SZ); //Save the chunk hash
		memcpy((void*)current_chunk->chunk_hash, (void*)s.song_md.song_hash, HASH_SZ); //Insert the song hash into the chunk hash space
		//Compute HASH
		sha256_compute_hash((void*)current_chunk , sizeof(song_chunk)-RSA_KEY_SZ , 1, hash_buffer); //Leave out signature during hash computation
		//Verify hash
		if( !compare_hashes((void*)chunk_hash_buffer, (void*)hash_buffer) )
		{
			mb_printf("Song data has been corrupted. Stopping on chunk %d \n\r", chunk_ct);
			return;
		}
		//Check that chunk_ct matches
		if(current_chunk->chunk_number != chunk_ct){
			mb_printf("Error: song chunk out of order. Stopping \n\r");
			return;
		}

		if(chunk_ct < PREVIEW_CHUNK_CT){
			if( !verify_song_hash_signature( chunk_hash_buffer)){
				print("Chunk signature verification failed \r\n");
				return;
			}
		}


        if( verify_song_hash_flag ){ //If we are doing the "last chunk first" check
            if(chunk_ct >= PREVIEW_CHUNK_CT) //Check the signature of the last chunk even if it is encrypted
            {
                rsa_begin_verify( (void*)current_chunk->chunk_hash_signature, (void*)GLOBAL_PUBLIC_E, (void*)GLOBAL_PUBLIC_N);
                if( !verify_song_hash_signature( chunk_hash_buffer)){
				    print("Chunk signature verification failed \r\n");
				    return;
			    }
            }
            verify_song_hash_flag = 0;
            chunk_ct = 0;
            continue; //Go back to the start of the song
        }

		//Write unencrypted data backout to song file
		Xil_MemCpy((void *)chunk_pointer, (void *)(current_chunk),(u32)sizeof(song_chunk));

		//chunk_pointer += sizeof(song_chunk);
		rem -= cp_num;
		chunk_ct+=1;
	}

    mb_printf("Song dump finished\r\n");
}

//////////////////////// MAIN ////////////////////////


int main() {
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

    // Congigure the DMA
    status = fnConfigDma(&sAxiDma);
    if(status != XST_SUCCESS) {
        mb_printf("DMA configuration ERROR\r\n");
        return XST_FAILURE;
    }

    // Start the LED
    //enableLED(led);
    set_stopped();

    // clear command channel
    memset((void*)c, 0, sizeof(cmd_channel));

    mb_printf("Audio DRM Module has Booted\n\r");

    // Handle commands forever
    while(1) {
        // wait for interrupt to start
        if (InterruptProcessed) {
            InterruptProcessed = FALSE;
            set_working();

            // c->cmd is set by the miPod player
            switch (c->cmd) {
            case LOGIN:
                login();
                break;
            case LOGOUT:
                logout();
                break;
            case QUERY_PLAYER:
                query_player();
                break;
            case QUERY_SONG:
                query_song();
                break;
            case SHARE:
                share_song();
                break;
            case PLAY:
                play_song();
                mb_printf("Done Playing Song\r\n");
                break;
            case DIGITAL_OUT:
                digital_out();
                break;
            default:
                break;
            }

            // reset statuses and sleep to allowe player to recognize WORKING state
            strcpy((char *)c->username, s.username);
            c->login_status = s.logged_in;
            usleep(500);
            set_stopped();
        }
    }

    cleanup_platform();
    return 0;
}

