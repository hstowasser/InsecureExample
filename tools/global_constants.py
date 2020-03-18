DEBUG = True

NUM_REGIONS = 32
NUM_USERS = 64

RSA_KEY_SZ = 128  # This is the size of our RSA key
RSA_KEY_SZ_BITS = RSA_KEY_SZ * 8


# Currently we generate 32 byte hashes but we use onl the 16 most significant bytes
HASH_SZ = 16  # This is the size of all hash values computed, and AES keys
HASH_SZ_BITS = HASH_SZ * 8
SONG_VERIFY_HASH_SZ = 32

USE_MD5 = 0
USE_SHA1 = 1
USE_SHA256 = 2
SONG_VERIFY_HASH_ALGORITHM = USE_SHA1

PAD_SZ = 12
CHUNK_SZ = 16000
ENC_CHUNK_SZ = 4 + PAD_SZ + CHUNK_SZ + HASH_SZ + RSA_KEY_SZ
ENABLED_REGION_BLOCK_SZ = 4+HASH_SZ*NUM_REGIONS
HEADER_SZ = 4 + 4 + ENABLED_REGION_BLOCK_SZ + HASH_SZ + SONG_VERIFY_HASH_SZ + HASH_SZ + RSA_KEY_SZ #first 4 is user ID plus padding, other 4 is song size

SONG_SHARING_HEADER_SZ = int(8 + (RSA_KEY_SZ * (NUM_USERS-1)))

PREVIEW_SZ = 30 * 48000 * 2




