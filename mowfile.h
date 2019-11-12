//THERE MAY BE OS SPECIFIC PATH DELIMITERS SUCH AS / & \
//CHANGE THE STRING TO COMPATIBLE VERSION
//GET THE FILES & FOLDERS INSIDE A DIRECTORY

//GLOBAL INCLUDES HERE

#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

//GENERAL DEFINES HERE
#define MOWFILEOK 1
#define MOWFILEERR 0

#define MOW_FILE_CHECK(x) if(x == MOWFILEERR) printf("\n%s %d MOWFILEERR\n",__FILE__,__LINE__)

//GENERAL DEBUG DEFINES HERE
#ifdef _DEBUG
#define MOW_FILE_ERROR(x) printf("\nERROR %s %d: %s\n",__FILE__,__LINE__,x);

#else
#define MOW_FILE_ERROR(x) ((void)0)
#endif

//IF ANDROID INCLUDE ANDROID ONLY LIBRARIES
#ifdef __ANDROID__
#include <jni.h>
#include <android/log.h>
#include "android/android_native_app_glue.h"

//ANDROID ONLY DEFINES HERE


//TODO:(kerem) MAYBE GUARD THIS DEFINE
#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "MOW", __VA_ARGS__);
#endif

//IF WINDOWS INCLUDE WINDOWS ONLY LIBRARIES
#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "User32.lib")
#define M_OS_DELIMITER_CHAR '\\'
#define M_OS_DELIMITER_STR "\\"
#define M_OS_FALSE_DELIMITER_CHAR '/'
#define M_OS_FALSE_DELIMITER_STR "/"

#ifdef _DEBUG


#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#else
//IF LINUX INCLUDE LINUX ONLY LIBRARIES
#define M_OS_DELIMITER_CHAR '/'
#define M_OS_DELIMITER_STR "/"
#endif

//DEFINE MOWFILE VARIABLES HERE

struct mowfile {
	uint64_t name_length;
	char* file_name;
	uint64_t content_length;
	char* content;
};

struct mowfolder {
	struct mowfolder* parrent_folder;
	uint64_t name_length;
	char* folder_name;
	uint64_t folder_count;
	struct mowfolder** folders;
	uint64_t file_count;
	struct mowfile** files;
	uint64_t folder_size;
};

//DEFINE MOWFILE FUNCTIONS HERE

struct mowfolder* m_read_folder(char* path);
struct mowfile* m_read_file(char* file_name);
uint16_t m_path_compatible(char* path);
void m_path_conv_compat(char* path);
int m_free_folder(struct mowfolder* folder);
int m_free_file(struct mowfile* file);
void m_folder_print(struct mowfolder* folder);

//IMPLEMENT MOW FUNCTIONS HERE

#ifdef _WIN32
struct mowfolder* m_read_folder(char* path) {
	if (NULL == path) {
		assert(path);
		return NULL;
	}


	//CHANGE THE CURRENT DIRECTORY TO PATH
	if (!SetCurrentDirectory(path)) {
		MOW_FILE_ERROR(path);
		goto MOW_FILE_RETURN;	//IF WE CAN'T SET THE PATH
	}
	size_t path_length = strlen(path);
	char tmp_file_name[MAX_PATH + 1];

	memcpy(tmp_file_name, path, path_length);
	tmp_file_name[path_length] = M_OS_DELIMITER_CHAR;
	tmp_file_name[path_length + 1] = '*';
	tmp_file_name[path_length + 2] = 0;
	WIN32_FIND_DATAA ffd;
	HANDLE h_find = FindFirstFileA(tmp_file_name, &ffd);
	if (INVALID_HANDLE_VALUE == h_find)
	{
		MOW_FILE_ERROR("Can't get first file inside the folder");
		goto MOW_FILE_RETURN;
	}
	//CREATE THE FOLDER STRUCT
	struct mowfolder* current_folder = calloc(sizeof(struct mowfolder), 1);
	assert(current_folder);
	if (!current_folder) {	//HANDLE IF WE CAN'T ALLOCATE
		goto MOW_FILE_CLOSE_HANDLE;
	}
	//TODO: FIRST GET THE CURRENT DIRECTORY THEN COMPARE WITH THE PATH 
	// THEN CHANGE THE WORKING DIRECTORY COMPARE WITH THE RESULTS BEFORE
	//THEN CREATE THE PATH WITH THE MINIMAL STRING OR USE THE OS RETURNED ONE
	current_folder->name_length = strlen(path);
	current_folder->folder_name = calloc(strlen(path) + 1, sizeof(char));
	current_folder->folder_size += current_folder->name_length;	//INCREASE THE FOLDER SIZE NOT 100% TRUE BUT WILL BE IMPORTANT
	if (!current_folder->folder_name) goto MOW_FILE_FREE_STRUCT;	//IF WE CAN'T ALLOCATE MEMORY
	memcpy(current_folder->folder_name, path, strlen(path));

	do
	{
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0) continue;
			current_folder->folder_count++;

			struct mowfolder** t = realloc(current_folder->folders, current_folder->folder_count * sizeof(struct mowfolder*));
			if (!t) {
				MOW_FILE_ERROR("CAN'T REALLOC");
				current_folder->folder_count--;
				goto MOW_FILE_FREE_STRUCT;
			}

			char tmp[MAX_PATH];	//CREATE THE TEMPORARY PATH NAME FOR RECURSIVE FUNCTION CALL
			memcpy(tmp, path, path_length);
			tmp[path_length] = M_OS_DELIMITER_CHAR;
			memcpy(tmp + path_length + 1, ffd.cFileName, strlen(ffd.cFileName));
			tmp[path_length + 1 + strlen(ffd.cFileName)] = 0;
			struct mowfolder* child_folder = m_read_folder(tmp);

			if (child_folder) {
				current_folder->folders = t;
				current_folder->folders[current_folder->folder_count - 1] = child_folder;
				child_folder->parrent_folder = current_folder;
				current_folder->folder_size += child_folder->folder_size + child_folder->name_length;//INCREASE THE FOLDER SIZE NOT 100% TRUE BUT WILL BE IMPORTANT
			}
			else {
				goto MOW_FILE_FREE_STRUCT;
			}


		}
		else
		{
			LARGE_INTEGER f_size;
			f_size.LowPart = ffd.nFileSizeLow;
			f_size.HighPart = ffd.nFileSizeHigh;
			uint64_t file_size = (uint64_t)f_size.QuadPart;
			uint64_t name_length = strlen(ffd.cFileName);
			current_folder->file_count++;
			struct mowfile** t = realloc(current_folder->files, current_folder->file_count * sizeof(struct mowfile*));
			if (!t) {
				MOW_FILE_ERROR("CAN'T REALLOC");
				current_folder->file_count--;
				goto MOW_FILE_FREE_STRUCT;
			}
			current_folder->files = t;
			struct mowfile* file = calloc(sizeof(struct mowfile), 1);
			if (!file) goto MOW_FILE_FREE_STRUCT;	//IF WE CAN'T ALLOCATE MEMORY
			file->file_name = calloc(name_length + 1, sizeof(char));
			if (!file->file_name) goto MOW_FILE_FREE_STRUCT;	//IF WE CAN'T ALLOCATE MEMORY
			file->content = calloc(file_size, sizeof(char));
			if (!file->content) goto MOW_FILE_FREE_STRUCT;	//IF WE CAN'T ALLOCATE MEMORY
			memcpy(file->file_name, ffd.cFileName, name_length);
			//GET THE FILE CONTENT
			file->content_length = file_size;

			FILE* f = fopen(ffd.cFileName, "rb");
			if (f) {
				size_t nb = 0;
				while (nb < file_size) {
					size_t tb = fread(file->content + nb, sizeof(char), (size_t)(file_size - nb), f);
					if (!tb) MOW_FILE_ERROR("FREAD");
					nb += tb;
				}
				fclose(f);
			}
			else {	//HANDLE CAN'T OPEN FILE FOR READ
				printf("Can't open file %s\n", ffd.cFileName);
				m_free_file(file);	//FREE THE FILE STRUCT
				current_folder->file_count--;	//DECREASE ALREADY INCREASED FILE COUNT
				continue;
			}

			current_folder->files[current_folder->file_count - 1] = file;
			current_folder->folder_size += file->content_length + file->name_length;//INCREASE THE FOLDER SIZE NOT 100% TRUE BUT WILL BE IMPORTANT
		}
	} while (FindNextFile(h_find, &ffd) != 0);

	DWORD dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES)
	{
		MOW_FILE_ERROR("FindFirstFile");
	}
	if (!FindClose(h_find)) MOW_FILE_ERROR("Can't close folder handler");
	return current_folder;

	//ERROR CONDITIONS RIGHT HERE BEFORE RETURN
MOW_FILE_FREE_STRUCT:
	m_free_folder(current_folder);
MOW_FILE_CLOSE_HANDLE:
	if (!FindClose(h_find)) MOW_FILE_ERROR("Can't close folder handler");
MOW_FILE_RETURN:
	return NULL;
}
#else
struct mowfolder* m_read_folder(char* path) {
	if (NULL == path) {
		assert(path);
		return NULL;
	}






	return NULL;
}
#endif

struct mowfile* m_read_file(char* file_name) {
	if (NULL == file_name) {
		assert(file_name);
		return NULL;
	}

	return NULL;
}

//CHECK IF THE PATH CONTAINS ANY DELIMITER THAT IS NOT OS SPECIFIC
//IF CONTAINS RETURN 1 IF NOT COMPATIBLE RETURN 0
uint16_t m_path_compatible(char* path) {	//TODO:(kerem) ADD VECTOR OPERATIONS FOR FASTER CHECK
	if (NULL == path) {
		assert(path);
		return 0;
	}
	size_t string_length = strlen(path);
	//TODO:(kerem) PAD THE BUFFER THAN MAKE VECTOR OPERATIONS FOR FATER SEARCH
	size_t i;
	for (i = 0; i < string_length; i++)
	{
		if (path[i] == M_OS_FALSE_DELIMITER_CHAR) return 0;
	}
	return 1;
}

//CONVERT THE OS PATH DELIMITERS
void m_path_conv_compat(char* path) {	//TODO:(kerem) ADD VECTOR OPERATIONS FOR FASTER CHECK
	if (NULL == path) {
		assert(path);
		return;
	}
	size_t string_length = strlen(path);
	//TODO:(kerem) PAD THE BUFFER THAN MAKE VECTOR OPERATIONS FOR FATER SEARCH
	size_t i;
	for (i = 0; i < string_length; i++)
	{
		if (path[i] == M_OS_FALSE_DELIMITER_CHAR) path[i] = M_OS_DELIMITER_CHAR;
	}
}



int m_free_folder(struct mowfolder* folder) {	//TODO:(kerem) CHANGE OR MAYBE ADD A NEW FUNCTION FOR ITERATIVELY FREEING
	assert(folder);
	if (folder) {	//TRAVERSE THE FOLDERS BY ITERATING
		for (uint64_t i = 0; i < folder->file_count; i++)
		{
			MOW_FILE_CHECK(m_free_file(folder->files[i]));
		}
		if (folder->files)free(folder->files);
		for (uint64_t i = 0; i < folder->folder_count; i++)
		{
			MOW_FILE_CHECK(m_free_folder(folder->folders[i]));
		}
		if (folder->folders)free(folder->folders);
		if (folder->folder_name)free(folder->folder_name);
		free(folder);
		return MOWFILEOK;
	}
	return MOWFILEERR;
}
int m_free_file(struct mowfile* file) {
	assert(file);
	if (file) {
		if (file->content)free(file->content);
		if (file->file_name)free(file->file_name);
		free(file);
		return MOWFILEOK;
	}return MOWFILEERR;
}

void m_folder_print(struct mowfolder* folder) {
	if (folder) {
#if 0
		for (uint64_t i = 0; i < folder->file_count; i++)
		{
			printf("%s %I64d\n", folder->files[i]->file_name, folder->files[i]->content_length);
}
#endif
		for (uint64_t i = 0; i < folder->folder_count; i++)
		{
			printf("<DIR> %s %I64d %I64d bytes\n", folder->folders[i]->folder_name, folder->folders[i]->folder_count + folder->folders[i]->file_count, folder->folders[i]->folder_size);
		}
		for (uint64_t i = 0; i < folder->folder_count; i++)
		{
			m_folder_print(folder->folders[i]);
		}

		printf("<DIR> %s %I64d %I64d\n", folder->folder_name, folder->folder_count, folder->folder_size);
	}
}