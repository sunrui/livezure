/*
 * read deep dir
 *
 * @author rui.sun 2013_1_4 13:32
 */
#include "readddir.h"
#include "readdir.h"

#include <sys/stat.h>
#include <io.h>

#ifdef WIN32
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#define getcwd _getcwd
#define chdir _chdir
#endif

void readddir(const char * path, void (* gotfile)(const char * path, const char* file))
{
	DIR * dir;
	struct dirent * entry;
	struct stat statbuf;
	char curdir[260];

	getcwd(curdir, 260);
	dir=opendir(path);
	chdir(path);

	while ((entry=readdir(dir))!= NULL)
	{
		stat(entry->d_name, &statbuf);

		if (strcmp(entry->d_name, ".") == 0 ||
			strcmp(entry->d_name, "..") == 0)
			continue;

		if (S_ISDIR(statbuf.st_mode))
		{
			char fullpath[260];
			sprintf(fullpath, "%s\\%s", path, entry->d_name);
			readddir(fullpath, gotfile);
			continue;
		}

		gotfile(path, entry->d_name);
	}

	closedir(dir);
	chdir(curdir);
}