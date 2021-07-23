//
// hash key util
//
// update 2012-6-19 rui.sun
//
#include "hashkey.h"

#include "sha.h"
#include "md5.h"

#include <time.h>
#include <string.h>
#include <stdio.h>

void gen_sha1_key(char * md, const char * inbuf, int inlen)
{
	SHA_CTX  ctx;

	if (inlen <= 0)
	{
		inlen = strlen(inbuf);
	}

	SHA_init(&ctx);
	SHA_update(&ctx, inbuf, inlen);
	memcpy(md, SHA_final(&ctx), 20);
}

void gen_md5_key(char * md, const char * inbuf, int inlen)
{
	MD5_CTX  ctx;

	if (inlen <= 0)
	{
		inlen = strlen(inbuf);
	}

	MD5_Init(&ctx);
	MD5_Update(&ctx, (unsigned char *)inbuf, inlen);
	MD5_Final(&ctx);
	memcpy(md, ctx.digest, 16);
}

void gen_hash_key(char * md, const char * inbuf, int inlen)
{
	char sha256[32] = { 0 };
	char md5[16] = { 0 };
	int i;

	// first, sha1 buffer first
	gen_sha1_key(sha256, inbuf, inlen);

	// then, md5 buffer second
	gen_md5_key(md5, sha256, sizeof(sha256));

	// third, generate hex data and return
	for (i = 0; i < 16; i++)
	{
		char hex[3];
		sprintf(hex, "%02X", (unsigned char)md5[i]);
		memcpy(md, hex, 2);
		(short *)md++;
	}
}

void gen_random_key(char * key)
{
	const char * curtime;
	time_t __time_t;

	time(&__time_t);
	curtime = ctime(&__time_t);

	gen_hash_key(key, curtime, -1);
}
