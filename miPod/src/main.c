/*
 * eCTF Collegiate 2020 miPod Example Code
 * Linux-side DRM driver
 */


#include "miPod.h"

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <linux/gpio.h>
#include <string.h>
#include <stdint.h>

volatile cmd_channel *c;

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include "HASH/sha256.h"
#include "HASH/md5.h"

//////////////////////// UTILITY FUNCTIONS ////////////////////////


// sends a command to the microblaze using the shared command channel and interrupt
void send_command(int cmd) {
    memcpy((void*)&c->cmd, &cmd, 1);

    //trigger gpio interrupt
    system("devmem 0x41200000 32 0");
    system("devmem 0x41200000 32 1");
}


// parses the input of a command with up to two arguments
// any arguments not present will be set to NULL
void parse_input(char *input, char **cmd, char **arg1, char **arg2) {
    *cmd = strtok(input, " \r\n");
    *arg1 = strtok(NULL, " \r\n");
    *arg2 = strtok(NULL, " \r\n");
}


// prints the help message while not in playback
void print_help() {
    mp_printf("miPod options:\r\n");
    mp_printf("  login <username> <pin>: log on to a miPod account (must be logged out)\r\n");
    mp_printf("  logout: log off of a miPod account (must be logged in)\r\n");
    mp_printf("  query <song.drm>: display information about the song\r\n");
    mp_printf("  share <song.drm> <username>: share the song with the specified user\r\n");
    mp_printf("  play <song.drm>: play the song\r\n");
    mp_printf("  digital_out <song.drm>: play the song to digital out\r\n");
    mp_printf("  exit: exit miPod\r\n");
    mp_printf("  help: display this message\r\n");
}


// prints the help message while in playback
void print_playback_help() {
    mp_printf("miPod playback options:\r\n");
    mp_printf("  stop: stop playing the song\r\n");
    mp_printf("  pause: pause the song\r\n");
    mp_printf("  resume: resume the paused song\r\n");
    mp_printf("  restart: restart the song\r\n");
    mp_printf("  ff: fast forwards 5 seconds(unsupported)\r\n");
    mp_printf("  rw: rewind 5 seconds (unsupported)\r\n");
    mp_printf("  help: display this message\r\n");
}


// loads a file into the song buffer with the associate
// returns the size of the file or 0 on error
size_t load_file(char *fname, char *song_buf) {
    int fd;
    struct stat sb;

    fd = open(fname, O_RDONLY);
    if (fd == -1){
        mp_printf("Failed to open file! Error = %d\r\n", errno);
        return 0;
    }

    if (fstat(fd, &sb) == -1){
        mp_printf("Failed to stat file! Error = %d\r\n", errno);
        return 0;
    }

    read(fd, song_buf, sb.st_size);
    close(fd);

    mp_printf("Loaded file into shared buffer (%dB)\r\n", sb.st_size);
    return sb.st_size;
}


//////////////////////// COMMAND FUNCTIONS ////////////////////////


// attempts to log in for a user
void login(char *username, char *pin) {
    if (!username || !pin) {
        mp_printf("Invalid user name/PIN\r\n");
        print_help();
        return;
    }

    // drive DRM
    strcpy((void*)c->username, username);
    strcpy((void*)c->pin, pin);
    send_command(LOGIN);
}


// logs out for a user
void logout() {
    // drive DRM
    send_command(LOGOUT);
}


// queries the DRM about the player
// DRM will fill shared buffer with query content
void query_player() {
    // drive DRM
    send_command(QUERY_PLAYER);
    while (c->drm_state == STOPPED) continue; // wait for DRM to start working
    while (c->drm_state == WORKING) continue; // wait for DRM to dump file

    // print query results
    mp_printf("Regions: %s", q_region_lookup(c->query, 0));
    for (int i = 1; i < c->query.num_regions; i++) {
        printf(", %s", q_region_lookup(c->query, i));
    }
    printf("\r\n");

    mp_printf("Authorized users: ");
    if (c->query.num_users) {
        printf("%s", q_user_lookup(c->query, 0));
        for (int i = 1; i < c->query.num_users; i++) {
            printf(", %s", q_user_lookup(c->query, i));
        }
    }
    printf("\r\n");
}


// queries the DRM about a song
void query_song(char *song_name) {
    // load the song into the shared buffer
    if (!load_file(song_name, (void*)&c->song)) {
        mp_printf("Failed to load song!\r\n");
        return;
    }

    // drive DRM
    send_command(QUERY_SONG);
    while (c->drm_state == STOPPED) continue; // wait for DRM to start working
    while (c->drm_state == WORKING) continue; // wait for DRM to finish

    // print query results

    mp_printf("Regions: %s", q_region_lookup(c->query, 0));
    for (int i = 1; i < c->query.num_regions; i++) {
        printf(", %s", q_region_lookup(c->query, i));
    }
    printf("\r\n");

    mp_printf("Owner: %s", c->query.owner);
    printf("\r\n");

    mp_printf("Authorized users: ");
    if (c->query.num_users) {
        printf("%s", q_user_lookup(c->query, 0));
        for (int i = 1; i < c->query.num_users; i++) {
            printf(", %s", q_user_lookup(c->query, i));
        }
    }
    printf("\r\n");
}


// attempts to share a song with a user
void share_song(char *song_name, char *username) {
    int fd;
    unsigned int length;
    ssize_t wrote, written = 0;

    if (!username) {
        mp_printf("Need song name and username\r\n");
        print_help();
    }

    // load the song into the shared buffer
    size_t file_len = load_file(song_name, (void*)&c->song);
    if ( !file_len) {
        mp_printf("Failed to load song!\r\n");
        return;
    }

    strcpy((char *)c->username, username);

    // drive DRM
    send_command(SHARE);
    while (c->drm_state == STOPPED) continue; // wait for DRM to start working
    while (c->drm_state == WORKING) continue; // wait for DRM to share song

    // request was rejected if WAV length is 0
    length = c->song.song_header.song_size;
    if (length == 0) {
        mp_printf("Share rejected\r\n");
        return;
    }

    // open output file
    fd = open(song_name, O_WRONLY);
    if (fd == -1){
        mp_printf("Failed to open file! Error = %d\r\n", errno);
        return;
    }

    // write song dump to file
    mp_printf("Writing song to file '%s' (%dB)\r\n", song_name, file_len);
    while (written < file_len) {
        wrote = write(fd, (char *)&c->song + written, file_len - written);
        if (wrote == -1) {
            mp_printf("Error in writing file! Error = %d\r\n", errno);
            return;
        }
        written += wrote;
    }
    close(fd);
    mp_printf("Finished writing file\r\n");
}


int sha256_verify( char * buf, int len)
{

	SHA256_CTX ctx;
	int pass = 1;

	sha256_init(&ctx);
	sha256_update(&ctx, (void*)buf, len);
	sha256_final(&ctx, (void*)c->song_verify_hash);
	mp_printf("Calculated SONG hash %d \n\r",len);
	for( int i = 0; i < 32; i++){
		printf("%02x ", c->song_verify_hash[i]);
	}
	printf("\n\r");
	c->song_length = len;

	return(pass);
}

int md5_verify( char * buf, int len)
{

	MD5_CTX ctx;
	int pass = 1;

	md5_init(&ctx);
	md5_update(&ctx, (void*)buf, len);
	md5_final(&ctx, (void*)c->song_verify_hash);
	mp_printf("Calculated SONG hash %d \n\r",len);
	c->song_length = len;

	return(pass);
}

// plays a song and enters the playback command loop
int play_song(char *song_name) {
    char usr_cmd[USR_CMD_SZ + 1], *cmd = NULL, *arg1 = NULL, *arg2 = NULL;

    // load song into shared buffer
    int file_size = load_file(song_name, (void*)&c->song);
    if (!file_size) {
        mp_printf("Failed to load song!\r\n");
        return 0;
    }

    c->song_length = 0; //Set song_length to 0


    // drive the DRM
    send_command(PLAY);

    md5_verify((void*)(&c->song.block_array), file_size - sizeof(c->song.song_header) - sizeof(c->song.shared_user_block) );


    //c->song_length = c->song.song_header.song_size;

    while (c->drm_state == STOPPED) continue; // wait for DRM to start playing

    // play loop
    while(1) {
        // get a valid command
        do {
            print_prompt_msg(song_name);
            fgets(usr_cmd, USR_CMD_SZ, stdin);

            // exit playback loop if DRM has finished song
            if (c->drm_state == STOPPED) {
                mp_printf("Song finished\r\n");
                return 0;
            }
        } while (strlen(usr_cmd) < 2);

        // parse and handle command
        parse_input(usr_cmd, &cmd, &arg1, &arg2);
        if (!cmd) {
            continue;
        } else if (!strcmp(cmd, "help")) {
            print_playback_help();
        } else if (!strcmp(cmd, "resume")) {
            send_command(PLAY);
            usleep(200000); // wait for DRM to print
        } else if (!strcmp(cmd, "pause")) {
            send_command(PAUSE);
            usleep(200000); // wait for DRM to print
        } else if (!strcmp(cmd, "stop")) {
            send_command(STOP);
            usleep(200000); // wait for DRM to print
            break;
        } else if (!strcmp(cmd, "restart")) {
            send_command(RESTART);
        } else if (!strcmp(cmd, "exit")) {
            mp_printf("Exiting...\r\n");
            send_command(STOP);
            return -1;
        } else if (!strcmp(cmd, "rw")) {
			send_command(RW);
			usleep(200000); // wait for DRM to print
        } else if (!strcmp(cmd, "ff")) {
			send_command(FF);
			usleep(200000); // wait for DRM to print
        } else if (!strcmp(cmd, "lyrics")) {
            mp_printf("Unsupported feature.\r\n\r\n");
            print_playback_help();
        } else {
            mp_printf("Unrecognized command.\r\n\r\n");
            print_playback_help();
        }
    }

    return 0;
}


// turns DRM song into original WAV for digital output
void digital_out(char *song_name) {
    char fname[64];

    // load song into shared buffer
    int file_size = load_file(song_name, (void*)&c->song);
    if (!file_size) {
        mp_printf("Failed to load song!\r\n");
        return;
    }

    // drive DRM
    send_command(DIGITAL_OUT);

    c->song_length = 0; //Set song_length to 0

	md5_verify((void*)(&c->song.block_array), file_size - sizeof(c->song.song_header) - sizeof(c->song.shared_user_block) );


    while (c->drm_state == STOPPED) continue; // wait for DRM to start working
    while (c->drm_state == WORKING) continue; // wait for DRM to dump file

    // open digital output file
    int wrote = 0, length = c->song_length;
    sprintf(fname, "%s.dout", song_name);
    int fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd == -1){
        mp_printf("Failed to open file! Error = %d\r\n", errno);
        return;
    }

    int chunk_ct = 0;
    int number_of_chunks = length%CHUNK_SZ==0 ? length/CHUNK_SZ : length/CHUNK_SZ + 1;
	int last_chunk_len = length%CHUNK_SZ; //The amount of samples in the last chunk. So we don't play junk at the end of the song

	uint8_t universal_buffer[sizeof(song_chunk)];


    mp_printf("Writing song to file '%s' (%dB)\r\n", fname, length);
    uint8_t * chunk_pointer = (uint8_t*)(&c->song.block_array);
	song_chunk * current_chunk = (void*)universal_buffer;

    while(number_of_chunks > chunk_ct) {
		// write song dump to file
    	memcpy((void*)current_chunk, (void *)(chunk_pointer),(uint32_t)(sizeof(song_chunk)));
		chunk_pointer += sizeof(song_chunk);

    	if( chunk_ct == number_of_chunks - 1){
    		wrote = write(fd, (void *)(current_chunk->data), (uint32_t)last_chunk_len); //Plus offset to get us to the data
    	}else{
    		wrote = write(fd, (void *)(current_chunk->data), CHUNK_SZ);
    	}
		if (wrote == -1) {
			mp_printf("Error in writing file! Error = %d \r\n", errno);
			return;
		}
		chunk_ct++;
    }
    close(fd);
    mp_printf("Finished writing file\r\n");
}


//////////////////////// MAIN ////////////////////////


int main(int argc, char** argv)
{
    int mem;
    char usr_cmd[USR_CMD_SZ + 1], *cmd = NULL, *arg1 = NULL, *arg2 = NULL;
    memset(usr_cmd, 0, USR_CMD_SZ + 1);

    // open command channel
    mem = open("/dev/uio0", O_RDWR);
    c = mmap(NULL, sizeof(cmd_channel), PROT_READ | PROT_WRITE,
             MAP_SHARED, mem, 0);
    if (c == MAP_FAILED){
        mp_printf("MMAP Failed! Error = %d\r\n", errno);
        return -1;
    }
    mp_printf("MiPod v0.1 \r\n");
    mp_printf("Command channel open at %p (%dB)\r\n", c, sizeof(cmd_channel));

    // dump player information before command loop
    query_player();

    // go into command loop until exit is requested
    while (1) {
        // get command
        print_prompt();
        fgets(usr_cmd, USR_CMD_SZ, stdin);

        // parse and handle command
        parse_input(usr_cmd, &cmd, &arg1, &arg2);
        if (!cmd) {
            continue;
        } else if (!strcmp(cmd, "help")) {
            print_help();
        } else if (!strcmp(cmd, "login")) {
            login(arg1, arg2);
        } else if (!strcmp(cmd, "logout")) {
            logout();
        } else if (!strcmp(cmd, "query")) {
            query_song(arg1);
        } else if (!strcmp(cmd, "play")) {
            // break if exit was commanded in play loop
            if (play_song(arg1) < 0) {
                break;
            }
        } else if (!strcmp(cmd, "digital_out")) {
            digital_out(arg1);
        } else if (!strcmp(cmd, "share")) {
            share_song(arg1, arg2);
        } else if (!strcmp(cmd, "exit")) {
            mp_printf("Exiting...\r\n");
            break;
        } else {
            mp_printf("Unrecognized command.\r\n\r\n");
            print_help();
        }
    }

    // unmap the command channel
    munmap((void*)c, sizeof(cmd_channel));

    return 0;
}
