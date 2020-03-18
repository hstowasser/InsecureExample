/*
 * constants.h
 *
 *  Created on: Feb 28, 2020
 *      Author: heiko
 */

#ifndef SRC_CONSTANTS_H_
#define SRC_CONSTANTS_H_

#include <stdint.h>
#include "xil_printf.h"
// shared DDR address
#define SHARED_DDR_BASE (0x20000000 + 0x1CC00000)


// number of seconds to record/playback
#define PREVIEW_TIME_SEC 30



#define FIFO_CAP (4096*4)
#define CHUNK_SZ 16000
#define HASH_SZ 16 // 128 bits. Only using most significant 16 bytes of SHA256
#define HASH_SZ_VERIFY 32
#define RSA_KEY_SZ 128
#define PAD_SZ 12

// ADC/DAC sampling rate in Hz
#define AUDIO_SAMPLING_RATE 48000
#define BYTES_PER_SAMP 2
#define PREVIEW_SZ (PREVIEW_TIME_SEC * AUDIO_SAMPLING_RATE * BYTES_PER_SAMP)
#define PREVIEW_CHUNK_CT (PREVIEW_SZ/CHUNK_SZ)

#define REGION_KEY_BOX_SZ HASH_SZ
#define NUMBER_OF_REGIONS 32

#define USER_KEY_BOX_SZ RSA_KEY_SZ
#define NUMBER_OF_SHARED_USERS 64 //64th block not used

#define get_padded_song_length(len) (len+(16-len%HASH_SZ)

// printing utility
#define MB_PROMPT "\r\nMB> "
#define mb_printf(...) xil_printf(MB_PROMPT __VA_ARGS__)

//#define NUM_REGIONS 32

// protocol constants
#define MAX_REGIONS 64
#define REGION_NAME_SZ 64
#define MAX_USERS 64
#define USERNAME_SZ 64
#define MAX_PIN_SZ 64
#define MAX_SONG_SZ (1<<27)  //Double check this

#define USE_SHA1_VERIFY
#ifdef USE_SHA1_VERIFY
	#define VERIFY_HASH_SZ 20
#else
	#define VERIFY_HASH_SZ 16
#endif

// LED colors and controller
struct color {
    u32 r;
    u32 g;
    u32 b;
};

typedef struct {
    int num_regions;
    int num_users;
    char owner[USERNAME_SZ];
    char regions[MAX_REGIONS * REGION_NAME_SZ]; // MAX_REGIONS was 64 by error
    char users[MAX_USERS * USERNAME_SZ];
} query;

#define q_region_lookup(q, i) (q.regions + (i * REGION_NAME_SZ))
#define q_user_lookup(q, i) (q.users + (i * USERNAME_SZ))



typedef struct __attribute__((__packed__)) {
	uint32_t regions; // Bits flipped for regions that are enabled
	uint8_t region_keys[REGION_KEY_BOX_SZ * NUMBER_OF_REGIONS];
} enabled_region_block;

typedef struct __attribute__((__packed__)) {
	uint8_t owner_id;//Owner
	uint8_t padding1;
	uint8_t padding2;
	uint8_t padding3;
	uint32_t song_size; //Song Size
	enabled_region_block enabled_regions; //Enabled Regions
	uint8_t song_hash[HASH_SZ]; //Song HASH
	uint8_t song_verify_hash[HASH_SZ_VERIFY];
	uint8_t header_hash[HASH_SZ]; //Added recently
	uint8_t hash_signature[RSA_KEY_SZ]; //HASH Signature
} header;

typedef struct __attribute__((__packed__)) {
	uint64_t enabled_users;
	uint8_t user_key_blocks[(MAX_USERS-1)][USER_KEY_BOX_SZ]; // User key blocks
} shared_users;


typedef struct __attribute__((__packed__)) {
	uint32_t chunk_number;
	uint8_t pad[PAD_SZ]; // Padded to multiple of 16. For AES
	uint8_t data[CHUNK_SZ];
	uint8_t chunk_hash[HASH_SZ]; //Data HASH
	uint8_t chunk_hash_signature[RSA_KEY_SZ]; //HASH Signature (unused for first 30 seconds)
} song_chunk;

typedef struct __attribute__((__packed__)) {
	header song_header;
	shared_users shared_user_block;
	uint8_t pad[sizeof(song_chunk)-sizeof(header)-sizeof(shared_users)];
	song_chunk block_array[]; //Data blocks
} song;


// struct to interpret shared command channel
typedef volatile struct __attribute__((__packed__)) {
    char cmd;                   // from commands enum
    char drm_state;             // from states enum
    char login_status;          // 0 = logged off, 1 = logged on
    char padding;               // not used
    char username[USERNAME_SZ]; // stores logged in or attempted username
    char pin[MAX_PIN_SZ];       // stores logged in or attempted pin
    int  song_length;
    char song_verify_hash[HASH_SZ_VERIFY];

    // shared buffer is either a drm song or a query
    union {
        song song;
        query query;
    };
} cmd_channel;




// store of internal state
typedef struct {
    char logged_in;             // whether or not a user is logged on
    u8 uid;                     // logged on user id
    char username[USERNAME_SZ]; // logged on username
    char pin[MAX_PIN_SZ];       // logged on pin
    u8 pin_hash[HASH_SZ];		// hash of user pin
    header song_md;            // current song metadata/header
    shared_users song_shared_user_md;
} internal_state;


#endif /* SRC_CONSTANTS_H_ */
