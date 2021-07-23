/*
 * sample symmetrical encryption support modified from internet
 * copyright (c) 2012 soho
 * author rui.sun <smallrui@live.com>
 * created 2012-5-1
 */
#include "encrypt.h"

#include "posixinc.h"
#include "aes.h"
#include "tea.h"

void simple_crypt(unsigned char * pdata, int len, unsigned long key0, unsigned long key1)
{
	union sDWORD
	{
		unsigned long ldata;
		unsigned char b[4];
	};

	union sWORD
	{
		short a;
		struct
		{
			unsigned char b0;
			unsigned char b1;
		}b;
	};

	union DISPLAY
	{
		struct{ unsigned char v; }B;
		struct {
			unsigned char b0 :1; unsigned char b1 :1; unsigned char b2 :1; unsigned char b3 :1; unsigned char b4 :1; unsigned char b5 :1; unsigned char b6 :1; unsigned char b7 :1;
		}b;
	};

	unsigned char * ps;
	union sDWORD p;
	union sDWORD k;
	union DISPLAY c;

	unsigned char RBKEY;
	unsigned char RBKEY1;
	unsigned char RBKEY2;
	unsigned char RBKEY3;

	char buffer1, buffer2;
	int count;
	int i;

	ps = pdata;
	len = len;

	p.ldata = key1;
	k.ldata = key0;
	RBKEY  =  p.b[0];
	RBKEY1 = p.b[3];
	RBKEY2 = p.b[1];
	RBKEY3 = p.b[2];
	RBKEY ^= key0;
	buffer1 = p.b[1];
	p.b[1] = k.b[3];
	buffer2 = p.b[3];
	p.b[3] = k.b[1];
	k.b[1] = buffer1;
	k.b[3] = buffer2;

	count = 0;
	while (len)
	{
		if (count == 8)
		{
			count = 0;
		}

		if (count < 4)
		{
			*(pdata++) ^= k.b[count];
		}
		else
		{
			*(pdata++) ^= p.b[count-4];
		}

		count++;
		len--;
	}

	for (i = 0; i < len; i++)
	{
		ps[i] = ps[i]^(RBKEY);
		c.B.v = ps[i];
		c.b.b0 ^= (RBKEY1&0x1);
		c.b.b2 ^= ((RBKEY1&0x2)>>1);
		c.b.b4 ^= ((RBKEY1&0x4)>>2);
		c.b.b6 ^= ((RBKEY1&0x8)>>3);
		c.b.b7 ^= ((RBKEY1&0x80)>>7);
		c.b.b1 ^= c.b.b3;
		c.b.b3 ^= c.b.b5;
		c.b.b5 ^= c.b.b7;
		c.b.b5 ^= p.b[0];
		c.b.b3 ^= p.b[1];
		c.b.b1 ^= p.b[2];
		c.b.b7 ^= p.b[3];
		c.b.b0 ^= (RBKEY2&0x1);
		c.b.b2 ^= ((RBKEY3&0x4)>>1);
		c.b.b4 ^= ((RBKEY2&0x8)>>3);
		c.b.b6 ^= ((RBKEY3&0x80)>>7);
	}
}

unsigned char * aes_crypt(const unsigned char * in, int inlen, int * outlen, const char * pubkey, const int enc)
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

	encrypt = (unsigned char *)calloc(len, sizeof(unsigned char));    
	AES_cbc_encrypt(in, encrypt, len, &aes, iv, enc ? AES_ENCRYPT : AES_DECRYPT);
	*outlen = len;

	return encrypt;
}

/*
 * tea encryption ported from gaim
 */
unsigned char * tea_crypt(unsigned char * in, int inlen, int * outlen, const unsigned char * pubkey, const int enc)
{
	unsigned char * out;

	*outlen = inlen + 32;
	out = (unsigned char *)malloc(*outlen);
	crypt(enc, in, inlen, (unsigned char *)pubkey, out, outlen);

	return out;
}