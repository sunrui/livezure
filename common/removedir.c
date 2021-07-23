#include "removedir.h"

#ifdef WIN32
#include <windows.h>
#include <shellapi.h>

bool remove_dir(char * path)
{
	SHFILEOPSTRUCTA FileOp;
	FileOp.fFlags = FOF_NOCONFIRMATION;
	FileOp.hNameMappings = NULL;
	FileOp.hwnd = NULL;
	FileOp.lpszProgressTitle = NULL;
	FileOp.pFrom = path;
	FileOp.pTo = NULL;
	FileOp.wFunc = FO_DELETE;
	return SHFileOperationA(&FileOp) == 0;
}
#else
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>

bool is_special_dir(const char *path)
{
	return strcmp(path, "..") == 0 || strcmp(path, ".") == 0;
}

bool is_dir(int attrib)
{
	return attrib == 16 || attrib == 18 || attrib == 20;
}

void show_error(const char *file_name)
{
	errno_t err;
	_get_errno(&err);
	switch(err)
	{
	case ENOTEMPTY:
		printf("given path is not a directory, the directory is not empty, or the directory is either the current working directory or the root directory.\n");
		break;
	case ENOENT:
		printf("path is invalid.\n");
		break;
	case EACCES:    
		printf("%s had been opened by some application, can't delete.\n", file_name);
		break;
	}
}

void get_file_path(const char *path, const char *file_name, char *file_path)
{
	strcpy(file_path, path);
	file_path[strlen(file_path) - 1] = '\0';
	strcat(file_path, file_name);
	strcat(file_path, "\\*");
}

bool remove_dir(char * path)
{
	struct _finddata_t dir_info;
	struct _finddata_t file_info;
	intptr_t f_handle;
	char tmp_path[260];

	if((f_handle = _findfirst(path, &dir_info)) != -1)
	{
		while(_findnext(f_handle, &file_info) == 0)
		{
			if(is_special_dir(file_info.name))
				continue;
				
			if(is_dir(file_info.attrib))
			{    
				get_file_path(path, file_info.name, tmp_path);
				remove_dir(tmp_path);
				tmp_path[strlen(tmp_path) - 2] = '\0';
				if(file_info.attrib == 20)
					printf("This is system file, can't delete!\n");
				else
				{
					if(_rmdir(tmp_path) == -1)
					{
						show_error(tmp_path);
					}
				}
			}
			else
			{
				strcpy(tmp_path, path);
				tmp_path[strlen(tmp_path) - 1] = '\0';
				strcat(tmp_path, file_info.name);
				if(remove(tmp_path) == -1)
				{
					show_error(file_info.name);
				}
			}
		}

		_findclose(f_handle);
	}
	else
	{
		show_error(path);
	}
	
	return _rmdir(path) != -1;
}
#endif
