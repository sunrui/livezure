#include "rpc_crypt.h"
#include "rpc_alloctor.h"

#include "aes.h"
#include "tea.h"

#include <assert.h>
#include <string.h>

unsigned char * rpc_aes_crypt(const unsigned char * in, int inlen, int * outlen, const unsigned char * pubkey, const int enc)
{
	unsigned char iv[16] = {
		0x35, 0x36, 0x32, 0x37, 0x41, 0x36, 0x41, 0x36, 0x36, 0x37, 0x41, 0x37, 0x37, 0x39, 0x42, 0x46
	};

	AES_KEY aes;
	unsigned char * encrypt;
	unsigned int len;

	len = 0;
	if ((inlen + 1) % AES_BLOCK_SIZE == 0) {
		len = inlen + 1;
	} else {
		len = ((inlen + 1) / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
	}

	if (enc == AES_ENCRYPT) {
		AES_set_encrypt_key((const unsigned char *)pubkey, 128, &aes);
	}
	else {
		AES_set_decrypt_key((const unsigned char *)pubkey, 128, &aes);
	}

	encrypt = (unsigned char *)rpc_calloc(len, sizeof(unsigned char));    
	AES_cbc_encrypt(in, encrypt, len, &aes, iv, enc ? AES_ENCRYPT : AES_DECRYPT);
	*outlen = len;

	return encrypt;
}

unsigned char * rpc_tea_crypt(unsigned char * in, int inlen, int * outlen, const unsigned char * pubkey, const int enc)
{
	unsigned char * out;

	*outlen = inlen + 32;
	out = (unsigned char *)rpc_malloc(*outlen);
	crypt(enc, in, inlen, (unsigned char *)pubkey, out, outlen);

	return out;
}