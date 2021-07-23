/*
 * string convert
 * @author rui.sun 2013_1_5 17_48
 */
#ifndef STRING_CONVERT_H
#define STRING_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <wchar.h>

wchar_t * mbs_to_wcs(const char* pszSrc);
char * wcs_to_mbs(const wchar_t * wcharStr);

wchar_t * utf8_to_utf16(const char *str_utf8);
char * utf16_to_utf8(const wchar_t *str_utf16);

#ifdef __cplusplus
}
#endif

#endif