/*
 * string convert
 * @author rui.sun 2012_11_25
 */
#include "string_convert.h"

#ifdef WIN32
#include <windows.h>
#endif

// code from http://www.ibm.com/developerworks/cn/linux/l-cn-ccppglb/index.html?ca=drs-cn-0408
wchar_t * mbs_to_wcs(const char* pszSrc){ 
	wchar_t* pwcs = NULL; 
	int size = 0; 
#if defined(_linux_) 
	setlocale(LC_ALL, "zh_CN.GB2312"); 
	size = mbstowcs(NULL,pszSrc,0); 
	pwcs = (wchar_t *)mallc(sizeof(wchar_t) * (size + 1));
	pwcs[size+1] = 0;
	pwcs[size] = 0;
	size = mbstowcs(pwcs, pszSrc, size+1);
	pwcs[size] = 0; 
#else
	size = MultiByteToWideChar(20936, 0, pszSrc, -1, 0, 0); 
	if(size <= 0) 
		return NULL; 
	pwcs = (wchar_t *)malloc(sizeof(wchar_t) * (size + 1));
	MultiByteToWideChar(20936, 0, pszSrc, -1, pwcs, size);
#endif
	return pwcs; 
}

char* wcs_to_mbs(const wchar_t * wcharStr){ 
	char* str = NULL; 
	int size = 0; 
#if defined(_linux_) 
	setlocale(LC_ALL, "zh_CN.UTF8"); 
	size = wcstombs( NULL, wcharStr, 0); 
	str = (char *)malloc(sizeof(char) * (size + 1)); 
	wcstombs( str, wcharStr, size); 
	str[size] = '\0'; 
#else
	size = WideCharToMultiByte(CP_UTF8, 0, wcharStr, -1, NULL, 0, NULL, NULL); 
	str = (char *)malloc(sizeof(char) * (size + 1)); 
	WideCharToMultiByte( CP_UTF8, 0, wcharStr, -1, str, size, NULL, NULL );
#endif 
	return str; 
}

static wchar_t * utf8_to_utf16(const char *str_utf8)
{
    int nchars = MultiByteToWideChar(CP_UTF8, 0, str_utf8, -1, NULL, 0);
    wchar_t *str_utf16 = (wchar_t *)malloc(nchars * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, str_utf8, -1, str_utf16, nchars);
    return str_utf16;
}

static char * utf16_to_utf8(const wchar_t *str_utf16)
{
    int nchars = WideCharToMultiByte(CP_UTF8, 0, str_utf16, -1, NULL, 0, 0, 0);
    char *str_utf8 = (char *)malloc(nchars * sizeof(char));
    WideCharToMultiByte(CP_UTF8, 0, str_utf16, -1, str_utf8, nchars, 0, 0);
    return str_utf8;
}