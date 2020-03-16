#!/usr/bin/env python3
"""
Description: Protects song by adding metadata or any other security measures
Use: Once per song
"""
import json
import struct
import os
import wave
from argparse import ArgumentParser
import numpy as np
import hashlib
from hashlib import sha256 , sha1, md5
from Crypto.PublicKey import RSA
from Crypto.Cipher import AES
import binascii
from Crypto.Util import asn1
import sys
from random import randint 
import array
from global_constants import *

def compute_hash(data):
    return sha256(data).digest()[0:HASH_SZ]

def unprotect_chunk__sample(song_chunk, song_hash, public_key):
    chunk_ct,  chunk, chunk_hash, hash_signature = struct.unpack('=I{pad_sz}x{chunk_sz}s{chunk_hash}s{song_hash_signature}s'.format(    
        pad_sz=PAD_SZ,
        chunk_sz=CHUNK_SZ,
        chunk_hash=HASH_SZ,
        song_hash_signature=RSA_KEY_SZ), 
        song_chunk)

    re_chunk = struct.pack('=I{pad_sz}x{chunk_sz}s{song_hash}s'.format(
        pad_sz=PAD_SZ,
        chunk_sz=CHUNK_SZ,
        song_hash=HASH_SZ),
        chunk_ct,
        chunk,
        song_hash)


    
    re_chunk_hash = compute_hash(re_chunk)    


    if( chunk_hash != re_chunk_hash):
        print("Chunk Mismatch")
        print(chunk_ct)
        print(chunk_hash.hex())
        print(chunk_hash_re.hex())

    chunk_hash_int = int.from_bytes(chunk_hash, byteorder='big', signed=False)
    hash_signature = int.from_bytes(hash_signature, byteorder='big', signed=False)

    # hash = int.from_bytes(hash_signature, byteorder='big')
    hash_signature_out = pow(hash_signature, public_key[1], public_key[0]) # 1 is e 0 is n

    #hash_signature_out = public_key.verify(chunk_hash,[(hash_signature),0])

    
    if hash_signature_out != chunk_hash_int:
        print("Signature verification failed")
        print('count ',chunk_ct)
        print('e= ', hex(public_key[1])) 
        print()
        print('n= ', hex(public_key[0]))
        print()
        print('signature= ',hex(hash_signature))
        print()
        print('sig out= ',hex(hash_signature_out))
        print()
        print('hash= ',chunk_hash_int)
        print('hash hex= ',chunk_hash.hex())
        exit()

    
    return chunk

def unprotect_chunk__encrypt(song_chunk, song_hash, aes_obj):
    chunk = aes_obj.decrypt(song_chunk)

    chunk_ct, song_chunk, chunk_hash, = struct.unpack('=I{pad_sz}x{chunk_sz}s{chunk_hash}s{song_hash_signature}x'.format(
        pad_sz=PAD_SZ,
        chunk_sz=CHUNK_SZ,
        chunk_hash=HASH_SZ,
        song_hash_signature=RSA_KEY_SZ), 
        chunk)

    re_chunk = struct.pack('=I{pad_sz}x{chunk_sz}s{song_hash}s'.format(
        pad_sz=PAD_SZ,
        chunk_sz=CHUNK_SZ,
        song_hash=HASH_SZ),
        chunk_ct,
        song_chunk,
        song_hash)
    

    chunk_hash_re = compute_hash(re_chunk)

    if chunk_hash_re != chunk_hash:
        print("HASH Mismatch")
        print(chunk_ct)
        print(chunk_hash.hex())
        print(chunk_hash_re.hex())

    return song_chunk

class unProtectedSong(object):
    """Example song object for protected song"""

    def __init__(self, path_to_song, metadata, song_hash, public_key, owner_key, common_key, song_size):
        """initialize values
        Args:
            path_to_song (string): file name where the song to be provisioned is stored
            metadata (bytearray): bytes containing metadata information
        """
        self.song = path_to_song
        #self.full_song, self.original_song = self.read_song(path_to_song)
        self.metadata = metadata
        self.song_hash = song_hash
        #self.private_key = private_key
        self.public_key = public_key
        self.owner_key = owner_key[0:HASH_SZ]
        self.common_key = common_key[0:HASH_SZ]
        self.song_size = song_size

    def save_unsecured_song_to_wave(self, file_location,song_verify_hash):
        fout = open(os.path.abspath(file_location), "wb")

        f = open(os.path.abspath(self.song), "rb")
        data = f.read()
        data = bytes(data)
        f.close()

        length = len(data)
        remaining = len(data) - HEADER_SZ - SONG_SHARING_HEADER_SZ


        #concat_key = self.owner_key+self.common_key
        #aes_key = compute_hash(concat_key)
        concat_song_key = compute_hash(self.owner_key + self.song_hash)
        aes_key = compute_hash(concat_song_key+self.common_key)
        print("owner_key: ", self.owner_key.hex())
        print("common_key: ", self.common_key.hex())
        print("AES Key: ",aes_key.hex())

        
        #PREVIEW_SZ = 30 * 48000 * 2
        PREVIEW_CT = int(PREVIEW_SZ / CHUNK_SZ)

        IV = binascii.unhexlify('000102030405060708090a0b0c0d0e0f')
        aes_obj = AES.new(aes_key, AES.MODE_CBC, IV)
        chunk_ct = 0

        remaining_decoded = self.song_size
        print("remaining_decoded", remaining_decoded)

        current = length - remaining

        #compute song_verify_hash
        if length%CHUNK_SZ != 0:
            uneven = 1
        else:
            uneven = 0
        padded_data_length = (int(remaining/CHUNK_SZ) + uneven)*ENC_CHUNK_SZ #TODO add last chunk if exists

        print("Padded length: ", remaining)
        if SONG_VERIFY_HASH_ALGORITHM == USE_MD5:
            reverified_song_hash = md5(data[HEADER_SZ+SONG_SHARING_HEADER_SZ: length]).digest()[0:SONG_VERIFY_HASH_SZ]
        elif SONG_VERIFY_HASH_ALGORITHM == USE_SHA1:
            reverified_song_hash = sha1(data[HEADER_SZ+SONG_SHARING_HEADER_SZ: length]).digest()[0:SONG_VERIFY_HASH_SZ]
        else:
            reverified_song_hash = sha256(data[HEADER_SZ+SONG_SHARING_HEADER_SZ: length]).digest()[0:SONG_VERIFY_HASH_SZ]
        
        if reverified_song_hash != song_verify_hash[0:len(reverified_song_hash)]:
            print("Song Hash not verified")
            print(reverified_song_hash.hex())
            print(song_verify_hash[0:len(reverified_song_hash)].hex())
        else:
            print("Song Hash verified")

        while remaining > ENC_CHUNK_SZ:
            current = length - remaining
            if(chunk_ct < PREVIEW_CT): 
                #preview
                
                chunk = unprotect_chunk__sample(data[current:(current+ENC_CHUNK_SZ)], self.song_hash, self.public_key)
                if len(chunk)!=CHUNK_SZ:
                    print("CHUNK SZ mismatch!!!")
                    print(len(chunk))
                fout.write(chunk)
            else:
                #other
                chunk = unprotect_chunk__encrypt(data[current:(current+ENC_CHUNK_SZ)], self.song_hash, aes_obj)
                
                if len(chunk)!=CHUNK_SZ:
                    print("CHUNK SZ mismatch!!!")
                fout.write(chunk)
            chunk_ct+=1
            remaining-=ENC_CHUNK_SZ
            remaining_decoded -=CHUNK_SZ
            
            
        current = length - remaining
        if remaining > 0:
            chunk = unprotect_chunk__encrypt(data[current:(current+ENC_CHUNK_SZ)], self.song_hash, aes_obj)

        print(remaining_decoded)
        fout.write(chunk[0:remaining_decoded])

            

    

    def read_song(self, path, metadata_frames=0):
        """Reads a wave file
        Args:
            path (string): path to song
            metadata_frames (int): if not 0 disregard this number of frames as metadata
        Returns:
            vals (array): integer array of decoded values
            song (Wave Object): wave object associated with entered song
        """
        song = wave.open(os.path.abspath(path), 'r')
        if metadata_frames:
            song.readframes(metadata_frames)  # skip the metadata frames when assigning vals
        vals = np.frombuffer(song.readframes(song.getnframes()), dtype=np.int16)
        song.close()
        return vals, song

# Attempts to return the common key
# region_key is the byte array of that section of the song file
# seems to work
def parse_enabled_regions(region_keys, region_info):
    enabled_regions = struct.unpack_from('=I', region_keys, offset=0)
    
    enabled_regions = enabled_regions[0]
    enabled_region_key_boxes = dict()
    for i in range(0,NUM_REGIONS):
        if ((enabled_regions >> i) & 1) == 1:
            start = 4+i*HASH_SZ # 4 is size of int32
            key_box = region_keys[start:start+16]
            enabled_region_key_boxes.update({i: key_box})
            

    region_secrets = json.load(open(os.path.abspath(region_info)))
    r = region_secrets['global']['regions']
    for region_name in r:
        region_key = binascii.unhexlify(r[region_name]["region_key"])
        region_num = int(r[region_name]["region"])
        if region_num in enabled_region_key_boxes:
            key_box = enabled_region_key_boxes[region_num]
            aes_encryptor = AES.new(region_key, AES.MODE_ECB)
            return True, bytes(aes_encryptor.decrypt(key_box))
    return False, [0]


# def parse_header(metadata):
#     ownerID, song_size, enabled_region_block, song_hash,header_hash, sig  = struct.unpack('=BxxxI{enabled_region_block}s{song_hash}s{header_hash}s{hash_signature}s'.format(
#         enabled_region_block=ENABLED_REGION_BLOCK_SZ,
#         song_hash=HASH_SZ,
#         header_hash=HASH_SZ,
#         hash_signature=RSA_KEY_SZ
#         ), metadata)
#     return ownerID, song_size, enabled_region_block, song_hash,header_hash, sig
def parse_header(metadata, public_key):
    ownerID, song_size, enabled_region_block, song_hash, song_verify_hash,header_hash, sig  = struct.unpack('=BxxxI{enabled_region_block}s{song_hash}s{song_verify_hash}s{header_hash}s{hash_signature}s'.format(
        enabled_region_block=ENABLED_REGION_BLOCK_SZ,
        song_hash=HASH_SZ,
        song_verify_hash=SONG_VERIFY_HASH_SZ,
        header_hash=HASH_SZ,
        hash_signature=RSA_KEY_SZ
        ), metadata)

    computed_header_hash = compute_hash(metadata[0:len(metadata)-HASH_SZ-RSA_KEY_SZ])
    
    hash_int = int.from_bytes(computed_header_hash, byteorder='big', signed=False)
    signature = int.from_bytes(sig, byteorder='big', signed=False)

    # hash = int.from_bytes(hash_signature, byteorder='big')
    hash_signature_out = pow(signature, public_key[1], public_key[0]) # 1 is e 0 is n

    if( hash_signature_out != hash_int):
        print("Header Signature Verification Failed")
        print(hex(public_key[1]))
        print(hex(public_key[0]))
        print(hex(signature))
        print(hex(hash_int))
        print(hex(hash_signature_out))
    else:
        print("Header Verified")

    if( computed_header_hash != header_hash):
        print("Header hash mismatch")
    
    if( DEBUG):
        print("Owner Id", ownerID)
        print("song_size", song_size)
        print("song_hash", song_hash.hex())
        print("song_verify_hash", song_verify_hash.hex())
        print("header_hash", header_hash.hex())
        print("song_signature", sig.hex())
        print("header len",len(metadata))
    return ownerID, song_size, enabled_region_block, song_hash, song_verify_hash,header_hash, sig


def main():
    parser = ArgumentParser(description='main interface to protect songs')
    parser.add_argument('--region-list', nargs='+', help='List of regions song can be played in', required=True)
    parser.add_argument('--region-secrets-path', help='File location for the region secrets file',
                        required=True)
    parser.add_argument('--outfile', help='path to save the protected song', required=True)
    parser.add_argument('--infile', help='path to unprotected song', required=True)
    parser.add_argument('--owner', help='owner of song', required=True)
    parser.add_argument('--user-secrets-path', help='File location for the user secrets file', required=True)
    args = parser.parse_args()

    regions = json.load(open(os.path.abspath(args.region_secrets_path)))

    #song_size = os.stat(os.path.abspath(args.infile)).st_size
    
    
    # global_private_key = binascii.unhexlify(regions["global"]["global_private"])

    # global_public_key = binascii.unhexlify(regions["global"]["global_public"])
    keyPair = RSA.generate(1024, os.urandom)
    keyPair.n = int.from_bytes(binascii.unhexlify(regions["global"]["global_key_n"]), byteorder='big', signed=False)
    keyPair.e = int.from_bytes(binascii.unhexlify(regions["global"]["global_key_e"]), byteorder='big', signed=False)
    keyPair.d = int.from_bytes(binascii.unhexlify(regions["global"]["global_key_d"]), byteorder='big', signed=False)

    # print("N ", keyPair.n)
    # exit()
    public_key = [keyPair.n , keyPair.e]
    if(DEBUG):
        print("Public n", hex(keyPair.n))

    
   

    user_secrets = json.load(open(os.path.abspath(args.user_secrets_path)))
    owner_key = binascii.unhexlify(user_secrets[args.owner]["pin_hash"])

    f =open(os.path.abspath(args.infile), "rb")
    metadata = f.read(HEADER_SZ)

    ownerID, song_size, enabled_region_block, song_hash, song_verify_hash ,header_hash, sig = parse_header(metadata, public_key)
    ret, common_key = parse_enabled_regions(enabled_region_block, args.region_secrets_path)
    
    if ret:
        print("Song Unlocked")
    else: 
        print("Song not Unlocked")
        exit()
    
    protected_song = unProtectedSong(args.infile, metadata,song_hash, public_key, owner_key, common_key, song_size)
    protected_song.save_unsecured_song_to_wave(args.outfile,song_verify_hash)


if __name__ == '__main__':
    main()

