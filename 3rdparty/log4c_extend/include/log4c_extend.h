/*
* public log provider
*
* author rui.sun 2012-7-3 Tue 5:03
*
* update
*    * for colored fputs rui.sun 2012-8-16 Thu 4:56
*/
#ifndef LOG4C_EXTEND_H
#define LOG4C_EXTEND_H

#ifdef  __cplusplus
# define __LOGC_BEGIN_DECLS  extern "C" {
# define __LOGC_END_DECLS    }
#else
# define __LOGC_BEGIN_DECLS
# define __LOGC_END_DECLS
#endif

__LOGC_BEGIN_DECLS

#define LOGD(...) logc_debug(__FILE__, \
	__FUNCDNAME__, \
	__LINE__, \
	__VA_ARGS__)

#define LOGI(...) logc_info(__FILE__, \
	__FUNCDNAME__, \
	__LINE__, \
	__VA_ARGS__)

#define LOGW(...) logc_warn(__FILE__, \
	__FUNCDNAME__, \
	__LINE__, \
	__VA_ARGS__)

#define LOGE(...) logc_error(__FILE__, \
	__FUNCDNAME__, \
	__LINE__, \
	__VA_ARGS__)

/*
 * log c init module
 *
 * @param[in] console title [valid in win32 only]
 * @param[in] localfile
 *     local file or ":memory:" tag
 */
void logc_init(const char * console_title, const char * localfile);

typedef enum e_log_evel
{
	log_level_debug = 3,
	log_level_info = 2,
	log_level_warn = 1,
	log_level_error = 0
} e_log_evel;

#ifdef WIN32
/*
 * log c show at debug trace
 */
void logc_set_debug_trace();
#endif

/*
 * set log c only show level lower in
 *
 * @param[in] level
 */
void logc_set_level(e_log_evel level);

void logc_debug(const char * file, const char * function, int line, const char * a_format, ...);
void logc_info(const char * file, const char * function, int line, const char * a_format, ...);
void logc_warn(const char * file, const char * function, int line, const char * a_format, ...);
void logc_error(const char * file, const char * function, int line, const char * a_format, ...);

void logc_fini();

#ifdef WIN32
#pragma comment(lib, "log4c.lib")
#ifdef _DEBUG
#pragma comment(lib, "log4c_extend_d.lib")
#else
#pragma comment(lib, "log4c_extend.lib")
#endif
#endif

__LOGC_END_DECLS

#endif