/*
 * adler32
 * copyright (c) 2012 soho
 * author rui.sun <smallrui@live.com>
 * created 2012-5-1
 */
#include "alder32.h"

/* algorithm from wiki */
#if 0
const int MOD_ADLER = 65521;

unsigned long adler32(const char *data, int len) /* where data is the location of the data in physical memory and 
													len is the length of the data in bytes */
{
	unsigned long a = 1, b = 0;
	int index;

	/* Process each byte of the data in order */
	for (index = 0; index < len; ++index)
	{
		a = (a + data[index]) % MOD_ADLER;
		b = (b + a) % MOD_ADLER;
	}

	return (b << 16) | a;
}
#endif

/**
 * Compute the Adler-32 checksum of a data stream.
 * This is a modified version based on adler32.c from the zlib library.
 *
 * Copyright (C) 1995 Mark Adler
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#define BASE 65521L /** largest prime smaller than 65536 */

#define DO1(buf)  {s1 += *buf++; s2 += s1;}
#define DO4(buf)  DO1(buf); DO1(buf); DO1(buf); DO1(buf);
#define DO16(buf) DO4(buf); DO4(buf); DO4(buf); DO4(buf);

#define adler 1
unsigned long adler32(const char * buf, int len)
{
    unsigned long s1 = adler & 0xffff;
    unsigned long s2 = adler >> 16;

    while (len>0) {
#if CONFIG_SMALL
        while(len>4 && s2 < (1U<<31)){
            DO4(buf); len-=4;
#else
        while(len>16 && s2 < (1U<<31)){
            DO16(buf); len-=16;
#endif
        }
        DO1(buf); len--;
        s1 %= BASE;
        s2 %= BASE;
    }
    return (s2 << 16) | s1;
}