/*
 * media public declare
 *
 * @author rui.sun 2012_12_30 11:40
 */
#ifndef MEDIA_API_PUBLIC_H
#define MEDIA_API_PUBLIC_H

#include "rpc_parser.h"

#define check_rid(id) \
{ \
	const char * c_##id; \
	c_##id = rpc_parser_get(req, "" #id "", NULL); \
	assert(c_##id != NULL); \
	id = atoi(c_##id != NULL ? c_##id : "-256"); \
}

#endif