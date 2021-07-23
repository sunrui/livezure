/*
 * read deep dir
 *
 * @author rui.sun 2013_1_4 13:32
 */
#ifndef READ_D_DIR_H
#define READ_D_DIR_H

#ifdef __cplusplus
extern "C" {
#endif

void readddir(const char * path, void (* gotfile)(const char * path, const char* file));

#ifdef __cplusplus
}
#endif

#endif