//
// hash key util
//
// update 2012-6-19 rui.sun
//
#ifndef HASH_KEY_H
#define HASH_KEY_H

/*
 * always 20 bits
 */
void gen_sha1_key(char * md, const char * inbuf, int inlen);

/*
 * always 16 bits
 */
void gen_md5_key(char * md, const char * inbuf, int inlen);

/*
 * gen 32 bits unique key, auto cal inbuf length if inlen <= 0
 */
void gen_hash_key(char * md, const char * inbuf, int inlen);

/*
 * gen random key, always 32 bits
 */
void gen_random_key(char * key);

#endif