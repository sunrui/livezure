/*
 *  modified for encoding/decoding url streams
 *  rui.sun 2012-7-17 Tue 8:01
 */
/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifndef URLBASE64_H
#define URLBASE64_h

unsigned char * urlbase64_encode(const unsigned char *src, int len,
			      int *out_len);
unsigned char * urlbase64_decode(const unsigned char *src, int len,
			      int *out_len);

#endif /* BASE64_H */
