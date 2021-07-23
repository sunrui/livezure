/*
 * rpc crypt
 * @copyright (c) livezure.com
 * @date 2013-4-15 23:54 rui.sun
 */
#ifndef RPC_CRYPT_H
#define RPC_CRYPT_H

/*
 * aes crypt
 */
unsigned char * rpc_aes_crypt(const unsigned char * in, int inlen, int * outlen, const unsigned char * pubkey, const int enc);

/*
 * tea crypt
 */
unsigned char * rpc_tea_crypt(unsigned char * in, int inlen, int * outlen, const unsigned char * pubkey, const int enc);

#endif