/*
 * main.c
 *
 *  Created on: Feb 19, 2020
 *      Author: heiko
 */
/*
 * eCTF Collegiate 2020 MicroBlaze Example Code
 * Audio Digital Rights Management
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

//#include "aes_decrypt_driver.h"
#include "AES/aes.h"

#include "HASH/sha256.h"

//#include "ECC/types.h"
#define uECC_WORD_SIZE 4
#include "ECC/uECC.h"

//////////////////////// GLOBALS ////////////////////////


// audio DMA access
static XAxiDma sAxiDma;

// LED colors and controller
u32 *led = (u32*) XPAR_RGB_PWM_0_PWM_AXI_BASEADDR;
const struct color RED =    {0x01ff, 0x0000, 0x0000};
const struct color YELLOW = {0x01ff, 0x01ff, 0x0000};
const struct color GREEN =  {0x0000, 0x01ff, 0x0000};
const struct color BLUE =   {0x0000, 0x0000, 0x01ff};

// change states
#define change_state(state, color) c->drm_state = state; setLED(led, color);
#define set_stopped() change_state(STOPPED, RED)
#define set_working() change_state(WORKING, YELLOW)
#define set_playing() change_state(PLAYING, GREEN)
#define set_paused()  change_state(PAUSED, BLUE)

// shared command channel -- read/write for both PS and PL
volatile cmd_channel *c = (cmd_channel*)SHARED_DDR_BASE;

static u8 universal_buffer[CHUNK_SZ];

// internal state store
internal_state s;


//////////////////////// INTERRUPT HANDLING ////////////////////////


// shared variable between main thread and interrupt processing thread
volatile static int InterruptProcessed = FALSE;
static XIntc InterruptController;

void myISR(void) {
    InterruptProcessed = TRUE;
}


//////////////////////// UTILITY FUNCTIONS ////////////////////////


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


// loads the song metadata in the shared buffer into the local struct
void load_song_md() {
    s.song_md.md_size = c->song.md.md_size;
    s.song_md.owner_id = c->song.md.owner_id;
    s.song_md.num_regions = c->song.md.num_regions;
    s.song_md.num_users = c->song.md.num_users;
    memcpy(s.song_md.rids, (void *)get_drm_rids(c->song), s.song_md.num_regions);
    memcpy(s.song_md.uids, (void *)get_drm_uids(c->song), s.song_md.num_users);
}
/*void load_song_md() {
	song * pointy_song = (song*)universal_buffer;
    s.song_md.md_size = pointy_song->md.md_size;
    s.song_md.owner_id = pointy_song->md.owner_id;
    s.song_md.num_regions = pointy_song->md.num_regions;
    s.song_md.num_users = pointy_song->md.num_users;
    memcpy(s.song_md.rids, (void *)get_drm_rids((*pointy_song)), s.song_md.num_regions);
    memcpy(s.song_md.uids, (void *)get_drm_uids((*pointy_song)), s.song_md.num_users);
}*/


// checks if the song loaded into the shared buffer is locked for the current user
int is_locked() {
    int locked = TRUE;

    // check for authorized user
    if (!s.logged_in) {
        mb_printf("No user logged in");
    } else {
        load_song_md();

        // check if user is authorized to play song
        if (s.uid == s.song_md.owner_id) {
            locked = FALSE;
        } else {
            for (int i = 0; i < NUM_PROVISIONED_USERS && locked; i++) {
                if (s.uid == s.song_md.uids[i]) {
                    locked = FALSE;
                }
            }
        }

        if (locked) {
            mb_printf("User '%s' does not have access to this song", s.username);
            return locked;
        }
        mb_printf("User '%s' has access to this song", s.username);
        locked = TRUE; // reset lock for region check

        // search for region match
        for (int i = 0; i < s.song_md.num_regions; i++) {
            for (int j = 0; j < (u8)NUM_PROVISIONED_REGIONS; j++) {
                if (PROVISIONED_RIDS[j] == s.song_md.rids[i]) {
                    locked = FALSE;
                }
            }
        }

        if (!locked) {
            mb_printf("Region Match. Full Song can be played. Unlocking...");
        } else {
            mb_printf("Invalid region");
        }
    }
    return locked;
}


// copy the local song metadata into buf in the correct format
// returns the size of the metadata in buf (including the metadata size field)
// song metadata should be loaded before call
int gen_song_md(char *buf) {
    buf[0] = ((5 + s.song_md.num_regions + s.song_md.num_users) / 2) * 2; // account for parity
    buf[1] = s.song_md.owner_id;
    buf[2] = s.song_md.num_regions;
    buf[3] = s.song_md.num_users;
    memcpy(buf + 4, s.song_md.rids, s.song_md.num_regions);
    memcpy(buf + 4 + s.song_md.num_regions, s.song_md.uids, s.song_md.num_users);

    return buf[0];
}



//////////////////////// COMMAND FUNCTIONS ////////////////////////


// attempt to log in to the credentials in the shared buffer
void login() {
    if (s.logged_in) {
        mb_printf("Already logged in. Please log out first.\r\n");
        memcpy((void*)c->username, s.username, USERNAME_SZ);
        memcpy((void*)c->pin, s.pin, MAX_PIN_SZ);
    } else {
        for (int i = 0; i < NUM_PROVISIONED_USERS; i++) {
            // search for matching username
            if (!strcmp((void*)c->username, USERNAMES[PROVISIONED_UIDS[i]])) {
                // check if pin matches
                if (!strcmp((void*)c->pin, PROVISIONED_PINS[i])) {
                    //update states
                    s.logged_in = 1;
                    c->login_status = 1;
                    memcpy(s.username, (void*)c->username, USERNAME_SZ);
                    memcpy(s.pin, (void*)c->pin, MAX_PIN_SZ);
                    s.uid = PROVISIONED_UIDS[i];
                    mb_printf("Logged in for user '%s'\r\n", c->username);
                    return;
                } else {
                    // reject login attempt
                    mb_printf("Incorrect pin for user '%s'\r\n", c->username);
                    memset((void*)c->username, 0, USERNAME_SZ);
                    memset((void*)c->pin, 0, MAX_PIN_SZ);
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
    if (c->login_status) {
        mb_printf("Logging out...\r\n");
        s.logged_in = 0;
        c->login_status = 0;
        memset((void*)c->username, 0, USERNAME_SZ);
        memset((void*)c->pin, 0, MAX_PIN_SZ);
        s.uid = 0;
    } else {
        mb_printf("Not logged in\r\n");
    }
}


// handles a request to query the player's metadata
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


// handles a request to query song metadata
void query_song() {
    char *name;

    // load song
    load_song_md();
    memset((void *)&c->query, 0, sizeof(query));

    c->query.num_regions = s.song_md.num_regions;
    c->query.num_users = s.song_md.num_users;

    // copy owner name
    uid_to_username(s.song_md.owner_id, &name, FALSE);
    strcpy((char *)c->query.owner, name);

    // copy region names
    for (int i = 0; i < s.song_md.num_regions; i++) {
        rid_to_region_name(s.song_md.rids[i], &name, FALSE);
        strcpy((char *)q_region_lookup(c->query, i), name);
    }

    // copy authorized uid names
    for (int i = 0; i < s.song_md.num_users; i++) {
        uid_to_username(s.song_md.uids[i], &name, FALSE);
        strcpy((char *)q_user_lookup(c->query, i), name);
    }

    mb_printf("Queried song (%d regions, %d users)\r\n", c->query.num_regions, c->query.num_users);
}


// add a user to the song's list of users
void share_song() {
    int new_md_len, shift;
    char new_md[256], uid;

    // reject non-owner attempts to share
    load_song_md();
    if (!s.logged_in) {
        mb_printf("No user is logged in. Cannot share song\r\n");
        c->song.wav_size = 0;
        return;
    } else if (s.uid != s.song_md.owner_id) {
        mb_printf("User '%s' is not song's owner. Cannot share song\r\n", s.username);
        c->song.wav_size = 0;
        return;
    } else if (!username_to_uid((char *)c->username, &uid, TRUE)) {
        mb_printf("Username not found\r\n");
        c->song.wav_size = 0;
        return;
    }

    // generate new song metadata
    s.song_md.uids[s.song_md.num_users++] = uid;
    new_md_len = gen_song_md(new_md);
    shift = new_md_len - s.song_md.md_size;

    // shift over song and add new metadata
    if (shift) {
        memmove((void *)get_drm_song(c->song) + shift, (void *)get_drm_song(c->song), c->song.wav_size);
    }
    memcpy((void *)&c->song.md, new_md, new_md_len);

    // update file size
    c->song.file_size += shift;
    c->song.wav_size  += shift;

    mb_printf("Shared song with '%s'\r\n", c->username);
}
void play_song() {
    u32 counter = 0, rem, cp_num, cp_xfil_cnt, offset, dma_cnt, length, *fifo_fill;

#ifdef TEST_ECC
    uint8_t private[32] = {0x8a, 0x45, 0xfe, 0x7a, 0xb0, 0x81, 0x75, 0xc3, 0x2d, 0x60, 0xac, 0x6f, 0xd4, 0xf8, 0xb8, 0x60, 0xa6, 0x1a, 0xc2, 0x65, 0xa5, 0xdc, 0xd2, 0x24, 0x3a, 0xc6, 0x0b, 0x0a, 0xb9, 0x04, 0x69, 0x4b};
	uint8_t public[64] =  {0x67, 0xc7, 0xd0, 0x34, 0x27, 0xac, 0xf6, 0x9f, 0xfe, 0x13, 0x9c, 0x25, 0x1b, 0x68, 0x28, 0x17, 0xb2, 0x5a, 0xbc, 0x77, 0x0f, 0x1c, 0xe4, 0x32, 0x13, 0xf1, 0x56, 0xfb, 0x23, 0x6e, 0x42, 0xc5, 0x23, 0xbb, 0xc3, 0x8f, 0x1f, 0x7d, 0x14, 0x0d, 0xd3, 0x4f, 0xbe, 0x07, 0x52, 0x9c, 0x9c, 0x8a, 0xe4, 0xff, 0x7b, 0x3d, 0x2a, 0xf9, 0xce, 0xed, 0xee, 0xf4, 0x46, 0x89, 0xa8, 0x3e, 0x3d, 0x35};

	uint8_t sig[64] = {0xd1, 0x84, 0xe3, 0xfc, 0xe8, 0xe9, 0xb4, 0xba, 0x3a, 0xbb, 0x48, 0x40, 0x89, 0x27, 0x84, 0x67, 0x2f, 0x3c, 0xf9, 0x64, 0x97, 0x2a, 0x2c, 0x57, 0x10, 0x50, 0x20, 0x68, 0xc8, 0xb6, 0x90, 0xd0, 0x77, 0x26, 0x47, 0xf1, 0x79, 0xcb, 0x61, 0xbc, 0x34, 0x36, 0x22, 0x6b, 0xf1, 0x71, 0x94, 0x73, 0xc1, 0x5c, 0xfc, 0x69, 0x6e, 0x82, 0xe4, 0xf9, 0xf9, 0xd2, 0x18, 0x90, 0xa9, 0x83, 0x99, 0xdd};
	uint8_t hash[32] = {0x67, 0xc7, 0xd0, 0x34, 0x27, 0xac, 0xf6, 0x9f, 0xfe, 0x13, 0x9c, 0x25, 0x1b, 0x68, 0x28, 0x17, 0xb2, 0x5a, 0xbc, 0x77, 0x0f, 0x1c, 0xe4, 0x32, 0x13, 0xf1, 0x56, 0xfb, 0x23, 0x6e, 0x42, 0xc5};
	const struct uECC_Curve_t * curves[5];
	int num_curves = 0;

    #if uECC_SUPPORTS_secp256k1
        curves[num_curves++] = uECC_secp256k1();
    #endif
#endif







    uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };

    uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };


    struct AES_ctx ctx;

	AES_init_ctx_iv(&ctx, key, iv);


	memcpy((void*)universal_buffer, (void *)(&c->song),(u32)(CHUNK_SZ));


	AES_CBC_decrypt_buffer(&ctx, (u8*)universal_buffer, CHUNK_SZ);  //Decrypt first chunk

	u8 * shared_pointer = (u8*)(&c->song);
	shared_pointer += CHUNK_SZ;


	song * pointy_song = (song*)universal_buffer;
	length = pointy_song->wav_size;
    mb_printf("Song length = %dB", length);

	mb_printf("Song is unlocked. Playing full song\r\n");

    rem = length;
    fifo_fill = (u32 *)XPAR_FIFO_COUNT_AXI_GPIO_0_BASEADDR;



    // write entire file to two-block codec fifo
    // writes to one block while the other is being played
    set_playing();
    while(rem > 0) {
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
            case RESTART:
                mb_printf("Restarting song... \r\n");
                rem = length; // reset song counter
                set_playing();
            default:
                break;
            }
        }

        // calculate write size and offset
        cp_num = (rem > CHUNK_SZ) ? CHUNK_SZ : rem;
        offset = (counter++ % 2 == 0) ? 0 : CHUNK_SZ;

		if(rem == length) //First
		{
			//This contains the correct data
			memcpy((u8*)(XPAR_MB_DMA_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + offset), (void*)universal_buffer,(u32)(cp_num));

		}else{
			memcpy((void *)(XPAR_MB_DMA_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + offset), (void *)(shared_pointer),(u32)(cp_num));

			shared_pointer += cp_num;
			AES_CBC_decrypt_buffer(&ctx, (u8*)(XPAR_MB_DMA_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + offset), (u32)(cp_num));

		}


#ifdef TEST_ECC
		sig[0]++;
		if (!uECC_verify(public, hash, sizeof(hash), sig, curves[0])) {
			set_playing();
		}else{
			set_paused();
		}
#endif




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
    }
}


/*void play_song() {
    u32 counter = 0, rem, cp_num, cp_xfil_cnt, offset, dma_cnt, length, *fifo_fill;

    uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };

    uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

    BYTE buf[SHA1_BLOCK_SIZE];

    struct AES_ctx ctx;

	AES_init_ctx_iv(&ctx, key, iv);

	//Xil_MemCpy( (void*)universal_buffer, (void *)(&c->song), CHUNK_SZ); //Load first chunk into buffer
	memcpy((void*)universal_buffer, (void *)(&c->song),(u32)(CHUNK_SZ));


	AES_CBC_decrypt_buffer(&ctx, (u8*)universal_buffer, CHUNK_SZ);  //Decrypt first chunk

	u8 * shared_pointer = (u8*)(&c->song);
	shared_pointer += CHUNK_SZ;


	song * pointy_song = (song*)universal_buffer;
	length = pointy_song->wav_size;
    mb_printf("Song length = %dB", length);

	mb_printf("Song is unlocked. Playing full song\r\n");

    rem = length;
    fifo_fill = (u32 *)XPAR_FIFO_COUNT_AXI_GPIO_0_BASEADDR;



    // write entire file to two-block codec fifo
    // writes to one block while the other is being played
    set_playing();
    while(rem > 0) {
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
            case RESTART:
                mb_printf("Restarting song... \r\n");
                rem = length; // reset song counter
                set_playing();
            default:
                break;
            }
        }

        // calculate write size and offset
        cp_num = (rem > CHUNK_SZ) ? CHUNK_SZ : rem;
        offset = (counter++ % 2 == 0) ? 0 : CHUNK_SZ;

        //memcpy((void*)universal_buffer, (void *)(&c->song + length - rem),(u32)(cp_num));
        //mb_printf("cp_num %d", cp_num);
		//Xil_MemCpy( (void*)universal_buffer, (void *)(get_drm_song(c->song) + length - rem), (u32)(cp_num)); //Load next chunk into buffer
		//mb_printf("First %x", universal_buffer[0]);
		//mb_printf("Last %x", universal_buffer[CHUNK_SZ-1]);
		//AES_CBC_decrypt_buffer(&ctx, (u8*)universal_buffer, (u32)(cp_num));
		//mb_printf("First %x", universal_buffer[0]);
		//mb_printf("Last %x", universal_buffer[CHUNK_SZ-1]);


		if(rem == length) //First
		{
			//This contains the correct data
			memcpy((u8*)(XPAR_MB_DMA_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + offset), (void*)universal_buffer,(u32)(cp_num));

		}else{
			//memcpy(universal_buffer, (void *)(&c->song + length - rem),(u32)(cp_num));
			memcpy(universal_buffer, (void *)(shared_pointer),(u32)(cp_num));
			shared_pointer += cp_num;
			AES_CBC_decrypt_buffer(&ctx, (u8*)universal_buffer, (u32)(cp_num));

			memcpy((void *)(XPAR_MB_DMA_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + offset), (void*)universal_buffer,(u32)(cp_num));

		}


		SHA1_CTX sha_ctx;

		sha1_init(&sha_ctx);
		sha1_update(&sha_ctx, universal_buffer, cp_num);
		sha1_final(&sha_ctx, buf);
		if(buf[0] > 5){
			set_playing();
		}else{
			set_paused();
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
    }
}*/
// plays a song and looks for play-time commands
/*
void play_song() {
    u32 counter = 0, rem, cp_num, cp_xfil_cnt, offset, dma_cnt, length, *fifo_fill;

    uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };

	uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };


	struct AES_ctx ctx;

	AES_init_ctx_iv(&ctx, key, iv);

	Xil_MemCpy( (void*)universal_buffer, (void *)(&c->song), CHUNK_SZ);

	AES_CBC_decrypt_buffer(&ctx, (u8*)universal_buffer, CHUNK_SZ);
	//AES_CBC_decrypt_copy(&ctx, (void *)(&c->song), (u8*)universal_buffer, CHUNK_SZ);

    mb_printf("Reading Audio Fileooo...");
    load_song_md();



    // get WAV length
    //length = c->song.wav_size;
    song * pointy_song = (song*)universal_buffer;
    length = pointy_song->wav_size;

    mb_printf("Song length = %dB", length);

    // truncate song if locked
    if (length > PREVIEW_SZ && is_locked()) {
        length = PREVIEW_SZ;
        mb_printf("Song is locked.  Playing only %ds = %dB\r\n",
                   PREVIEW_TIME_SEC, PREVIEW_SZ);
    } else {
        mb_printf("Song is unlocked. Playing full song\r\n");
    }

    rem = length;
    //rem = length - CHUNK_SZ;
    fifo_fill = (u32 *)XPAR_FIFO_COUNT_AXI_GPIO_0_BASEADDR;

    // write entire file to two-block codec fifo
    // writes to one block while the other is being played
    set_playing();

    int first = 1;

    while(rem > 0) {
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
            case RESTART:
                mb_printf("Restarting song... \r\n");
                rem = length; // reset song counter
                set_playing();
            default:
                break;
            }
        }

        // calculate write size and offset
        cp_num = (rem > CHUNK_SZ) ? CHUNK_SZ : rem;
        offset = (counter++ % 2 == 0) ? 0 : CHUNK_SZ;

        if(first)
        {
        	Xil_MemCpy((void *)(XPAR_MB_DMA_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + offset),
					   (void *)(universal_buffer),
					   (u32)(CHUNK_SZ));
        	first = 0;

        }else{
        	// do first mem cpy here into DMA BRAM
			Xil_MemCpy((void *)(universal_buffer),
					   (void *)(get_drm_song(c->song) + length - rem),
					   (u32)(cp_num));

			AES_CBC_decrypt_buffer(&ctx, (u8*)universal_buffer, (u32)(cp_num));

			Xil_MemCpy((void *)(XPAR_MB_DMA_AXI_BRAM_CTRL_0_S_AXI_BASEADDR ),
					   (void *)(universal_buffer),
					   (u32)(cp_num));

        }
        //AES_CBC_decrypt_buffer(&ctx, (u8*)(XPAR_MB_DMA_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + offset), (u32)(cp_num));

        cp_xfil_cnt = cp_num;
        set_working();

        while (cp_xfil_cnt > 0) {

            // polling while loop to wait for DMA to be ready
            // DMA must run first for this to yield the proper state
            // rem != length checks for first run
            while (XAxiDma_Busy(&sAxiDma, XAXIDMA_DMA_TO_DEVICE) && rem != length && *fifo_fill < (FIFO_CAP - 32));
        	//while (XAxiDma_Busy(&sAxiDma, XAXIDMA_DMA_TO_DEVICE) && (!first) && *fifo_fill < (FIFO_CAP - 32));

            // do DMA
            dma_cnt = (FIFO_CAP - *fifo_fill > cp_xfil_cnt)
                      ? FIFO_CAP - *fifo_fill
                      : cp_xfil_cnt;
            fnAudioPlay(sAxiDma, offset, dma_cnt);
            cp_xfil_cnt -= dma_cnt;
        }
        rem -= cp_num;
    }
}*/


// removes DRM data from song for digital out
void digital_out() {
    // remove metadata size from file and chunk sizes
    c->song.file_size -= c->song.md.md_size;
    c->song.wav_size -= c->song.md.md_size;

    if (is_locked() && PREVIEW_SZ < c->song.wav_size) {
        mb_printf("Only playing 30 seconds");
        c->song.file_size -= c->song.wav_size - PREVIEW_SZ;
        c->song.wav_size = PREVIEW_SZ;
    }

    // move WAV file up in buffer, skipping metadata
    mb_printf(MB_PROMPT "Dumping song (%dB)...", c->song.wav_size);
    memmove((void *)&c->song.md, (void *)get_drm_song(c->song), c->song.wav_size);

    mb_printf("Song dump finished\r\n");
}

/*
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
    enableLED(led);
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
}*/


int main(void)
{
	enableLED(led);

	//uint8_t tc2_1[64] = {0};
	uint8_t res2_1[32] = {0x85, 0xe6, 0x55, 0xd6, 0x41, 0x7a, 0x17, 0x95, 0x33, 0x63, 0x37, 0x6a, 0x62, 0x4c, 0xde, 0x5c, 0x76, 0xe0, 0x95, 0x89, 0xca, 0xc5, 0xf8, 0x11, 0xcc, 0x4b, 0x32, 0xc1, 0xf2, 0x0e, 0x53, 0x3a};
	uint8_t tc2_1[64] = { 0x61, 0x62, 0x63, 0x64, 0x62, 0x63, 0x64, 0x65, 0x63, 0x64, 0x65, 0x66, 0x64, 0x65, 0x66, 0x67, 0x65, 0x66
			, 0x67, 0x68, 0x66, 0x67, 0x68, 0x69, 0x67, 0x68, 0x69, 0x6A, 0x68, 0x69, 0x6A, 0x6B, 0x69, 0x6A, 0x6B, 0x6C, 0x6A, 0x6B
			, 0x6C, 0x6D, 0x6B, 0x6C, 0x6D, 0x6E, 0x6C, 0x6D, 0x6E, 0x6F, 0x6D, 0x6E, 0x6F, 0x70, 0x6E, 0x6F, 0x70, 0x71, 0x80, 0x00
			, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	//uint8_t res2_1[32] = {0x85, 0xE6, 0x55, 0xD6, 0x41, 0x7A, 0x17, 0x95, 0x33, 0x63, 0x37, 0x6A, 0x62, 0x4C, 0xDE
	//		, 0x5C, 0x76, 0xE0, 0x95, 0x89, 0xCA, 0xC5, 0xF8, 0x11, 0xCC, 0x4B, 0x32, 0xC1, 0xF2, 0x0E, 0x53, 0x3A};

//	uint8_t text1[] = {"abc"};
	uint8_t hash_out[32];


	sha256_compute_hash( tc2_1, 64, 1, hash_out);


	for(int i = 0; i < 32; i++)
		printf("%02x", hash_out[i]);
	printf("\n");

	int pass = memcmp(res2_1, hash_out, SHA256_HASH_SZ-1); //returns 0 if the same
	printf("Pass = %d\n", pass);
	if(pass==0){
		set_playing();
	}else{
		set_paused();
	}
}

/*
int main(void){
	u32 test=  0x6bc1bee2;
	enableLED(led);


	int pass = 1;

	for (int i = 0; i < 64; i+=4)
	{
		test++;
		Xil_Out32(SHA_BASEADDR + SHA_BLOCK_OFFSET + i, test);
		u32 test_o = Xil_In32(SHA_BASEADDR + SHA_BLOCK_OFFSET + i);
		if(test_o != test)
		{
			pass = 0;
		}
	}
	if(pass==0){
		set_playing();
		return 0;
	}
	for (int i = 0; i < SHA256_HASH_SZ; i+=4)
	{
		test++;
		Xil_Out32(SHA_BASEADDR + SHA_DIGEST_OFFSET + i, test);
		u32 test_o = Xil_In32(SHA_BASEADDR + SHA_DIGEST_OFFSET + i);
		if(test_o == test) //Readback should be different
		{
			pass = 0;
		}
	}

	if(pass==1){
		set_playing();
	}else{
		set_paused();
	}

}*/


// PASS
//#define READBACK_TEST
#ifdef READBACK_TEST

int main(void){
	u32 test=  0x6bc1bee2;
	enableLED(led);


	int pass = 1;

	for (int i = 0; i < AES_keyExpSize + 16; i+=4)
	{
		test++;
		Xil_Out32(AES_DECRYPT_BASEADDR + BASE_KEY_REGISTERS_OFFSET + i, test);
		u32 test_o = Xil_In32(AES_DECRYPT_BASEADDR + BASE_KEY_REGISTERS_OFFSET + i);
		if(test_o != test)
		{
			pass = 0;
		}
	}
	if(pass==0){
		set_playing();
		return 0;
	}
	for (int i = 0; i < 16; i+=4)
	{
		test++;
		Xil_Out32(AES_DECRYPT_BASEADDR + BASE_DATA_OUT_REGISTER_OFFSET + i, test);
		u32 test_o = Xil_In32(AES_DECRYPT_BASEADDR + BASE_DATA_OUT_REGISTER_OFFSET + i);
		if(test_o == test) //Readback should be different
		{
			pass = 0;
		}
	}

	if(pass==1){
		set_playing();
	}else{
		set_paused();
	}

}
#endif



