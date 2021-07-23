/*
 * rpc shell
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-7-31 Tue 10:41
 */
#ifndef RPC_SHELL_T
#define RPC_SHELL_T

#include <stdint.h> /* for uint64_t, uint32_t */

/* rpc protocol magic number */
#define RPC_MAGIC_NUMBER 0x6C6976657A757265 /* livezure */

/* rpc default version sign */
#define RPC_VERSION_SIGN 0x57

#ifdef WIN32
#pragma pack(push, 1)
#endif
typedef struct rpc_shell
{
	uint64_t magic;				/* magic code, always RPC_MAGIC_NUMBER */
	uint32_t version:8;			/* protocol version always be 0 */
	uint32_t category:8;		/* message category */
	uint32_t sub_category:8;	/* sub message category */
	rpc_req_type req:2;			/* request type */
	rpc_crypt_type crypt:6;		/* encrypt type */
	rpc_size_t body_size;		/* encrypt-ed buffer size */
#ifdef RPC_SHELL_CHECKSUM		/* not used current */
	unsigned long checksum;		/* buffer checksum in alder32 */
	int sequence;				/* default sequence number, reserved, must be 0 */
#endif
	int reserved;				/* reserved, always be 0 */
} rpc_shell_t
#ifdef _linux_
	__attribute__((aligned(1)))
#endif
	;
#ifdef WIN32
#pragma pack(pop)
#endif

#endif