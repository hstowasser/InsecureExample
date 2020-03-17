/*
 * miPod.h
 *
 *  Created on: Jan 9, 2020
 *      Author: ectf
 */

#ifndef SRC_MIPOD_H_
#define SRC_MIPOD_H_

#include <stdint.h>

// miPod constants
#define USR_CMD_SZ 64

// protocol constants
#define MAX_REGIONS 64
#define REGION_NAME_SZ 64
#define MAX_USERS 64
#define USERNAME_SZ 64
#define MAX_PIN_SZ 64
#define MAX_SONG_SZ  (1<<25)

#define CHUNK_SZ 16000
#define HASH_SZ 16 // 128 bits. Only using most significant 16 bytes of SHA256
#define HASH_SZ_VERIFY 32
#define RSA_KEY_SZ 128
#define PAD_SZ 12

#define REGION_KEY_BOX_SZ HASH_SZ
#define NUMBER_OF_REGIONS 32
#define NUM_REGIONS 32

#define USER_KEY_BOX_SZ RSA_KEY_SZ
#define NUMBER_OF_SHARED_USERS 64 //64th block not used

#define get_padded_song_length(len) (len+(16-len%HASH_SZ)

// printing utility
#define MP_PROMPT "mP> "
#define mp_printf(...) printf(MP_PROMPT __VA_ARGS__)

#define USER_PROMPT "miPod %s# "
#define print_prompt() printf(USER_PROMPT, "")
#define print_prompt_msg(...) printf(USER_PROMPT, __VA_ARGS__)

// struct to interpret shared buffer as a query
typedef struct {
    int num_regions;
    int num_users;
    char owner[USERNAME_SZ];
    char regions[MAX_REGIONS * REGION_NAME_SZ];
    char users[MAX_USERS * USERNAME_SZ];
} query;

// simulate array of 64B names without pointer indirection
#define q_region_lookup(q, i) (q.regions + (i * REGION_NAME_SZ))
#define q_user_lookup(q, i) (q.users + (i * USERNAME_SZ))


// struct to interpret drm metadata
typedef struct __attribute__((__packed__)) {
    char md_size;
    char owner_id;
    char num_regions;
    char num_users;
    char buf[];
} drm_md;



typedef struct __attribute__((__packed__)) {
	uint32_t regions; // Bits flipped for regions that are enabled
	uint8_t region_keys[REGION_KEY_BOX_SZ * NUM_REGIONS];
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

// accessors for variable-length metadata fields
#define get_drm_rids(d) (d.md.buf)
#define get_drm_uids(d) (d.md.buf + d.md.num_regions)
#define get_drm_song(d) ((char *)(&d.md) + d.md.md_size)


// shared buffer values
enum commands { QUERY_PLAYER, QUERY_SONG, LOGIN, LOGOUT, SHARE, PLAY, STOP, DIGITAL_OUT, PAUSE, RESTART, FF, RW };
enum states   { STOPPED, WORKING, PLAYING, PAUSED };


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
        char buf[MAX_SONG_SZ]; // sets correct size of cmd_channel for allocation
    };
} cmd_channel;

#endif /* SRC_MIPOD_H_ */
