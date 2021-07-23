/*
 * urlencode/decode helper
 * port from http://www.geekhideout.com/urlcode.shtml
 *
 * rui.sun 2012-7-5 Thu 17:42
 */
#ifndef URLCODE_H
#define URLCODE_H

/* Converts a hex character to its integer value */
char from_hex(char ch);

/* Converts an integer value to its hex character*/
char to_hex(char code);

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(char *str);

/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_decode(char *str);

#endif