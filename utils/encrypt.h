/*
 * sample symmetrical encryption support modified from internet
 * copyright (c) 2012 soho
 * author rui.sun <smallrui@live.com>
 * created 2012-5-1
 */
#ifndef ENCRYPT_H
#define ENCRYPT_H

/*
 * symmetrical encryption support modified from internet
 */
void simple_crypt(unsigned char * pdata, int len, unsigned long key0, unsigned long key1);

/*
 * aes encryption from openssl
 */
unsigned char * aes_crypt(const unsigned char * in, int inlen, int * outlen, const char * pubkey, const int enc);

/*
 * tea encryption ported from gaim
 */
unsigned char * tea_crypt(unsigned char * in, int inlen, int * outlen, const unsigned char * pubkey, const int enc);

#endif