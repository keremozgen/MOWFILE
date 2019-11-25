//THERE MAY BE OS SPECIFIC PATH DELIMITERS SUCH AS // & \\
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

#define MOW_FILE_LINE() printf(" %s %d\n",__FILE__,__LINE__)
#define MOW_FILE_CHECK(x) if(x == MOWFILEERR) printf("\n%s %d MOWFILEERR\n",__FILE__,__LINE__)
#define MOW_FILE_STRERROR() printf("\n%s %s %d\n",strerror(errno),__FILE__,__LINE__)
#define MOW_FILE_ERR_CHECK(x) if(x == MOWFILEERR) printf("\n%s %s %d\n",strerror(errno),__FILE__,__LINE__)
#define MOW_FILE_ERR_PARAM_STR(x,y) if(x == MOWFILEERR) printf("\n%s \"%s\" %s %d\n",strerror(errno),y,__FILE__,__LINE__)
#define MOW_FILE_ERR_CHECK_PARAM_STR(x,y) assert(x);if(x == MOWFILEERR) printf("\n%s \"%s\" %s %d\n",strerror(errno),y,__FILE__,__LINE__)

//GENERAL DEBUG DEFINES HERE
#ifdef _DEBUG
#define MOW_FILE_ERROR(x) printf("\nERROR %s %d: %s\n",__FILE__,__LINE__,x);

#else
#define MOW_FILE_ERROR(x) ((void)0)
#endif

//IF ANDROID INCLUDE ANDROID ONLY LIBRARIES
#if defined(__ANDROID__) && !defined(ANDROIDPRINT)
#define ANDROIDPRINT
#include <jni.h>
#include <android/log.h>
#include "android/android_native_app_glue.h"

//ANDROID ONLY DEFINES HERE


#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "MOW", __VA_ARGS__);
#endif

//IF WINDOWS INCLUDE WINDOWS ONLY LIBRARIES
#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "User32.lib")
#include <io.h>
#include <direct.h>

#define M_PATH_MAX MAX_PATH
#ifdef _DEBUG


#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#else
//IF LINUX AND ANDROID INCLUDE LINUX AND ANDROID ONLY LIBRARIES
#include <unistd.h>

#ifndef __EMSCRIPTEN__

#include <linux/limits.h>
#include <malloc.h> //IS THIS INCLUDE NEEDED IN WINDOWS BECAUSE OF WINDOWS.H ?
#else
#include <emscripten.h>
#define PATH_MAX 256
#endif

#define M_PATH_MAX PATH_MAX

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#endif
#if (!defined(M_LIN_DELIMITER) && defined(_WIN32))
#define M_OS_DELIMITER_CHAR '\\'
#define M_OS_DELIMITER_STR "\\"
#define M_OS_FALSE_DELIMITER_CHAR '/'
#define M_OS_FALSE_DELIMITER_STR "/"
#elif defined(M_LIN_DELIMITER) || ( defined(__linux__) || defined(__EMSCRIPTEN__))
#define M_OS_DELIMITER_CHAR '/'
#define M_OS_DELIMITER_STR "/"
#define M_OS_FALSE_DELIMITER_CHAR '\\'
#define M_OS_FALSE_DELIMITER_STR "\\"
#define M_LIN_DELIMITER
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

struct mowfolder* m_read_folder(const char* path);

struct mowfile* m_read_file(const char* file_name);

int m_path_compatible(char* path);

void m_path_conv_compat(char* path);

int m_free_folder(struct mowfolder* folder);

int m_free_file(struct mowfile* file);

void m_folder_print(struct mowfolder* folder);

int m_write_folder(char* abs_path, struct mowfolder* folder);

char* m_get_current_dir(void);

int m_set_current_dir(const char* abs_path);

int m_dir_exist(const char* abs_path);

int m_create_dir(char* abs_path);


//IMPLEMENT MOW FUNCTIONS HERE

#ifdef _WIN32
struct mowfolder* m_read_folder(const char* path) {
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
	struct mowfolder* current_folder = (struct mowfolder*)calloc(sizeof(struct mowfolder), 1);
	assert(current_folder);
	if (!current_folder) {	//HANDLE IF WE CAN'T ALLOCATE
		goto MOW_FILE_CLOSE_HANDLE;
	}
	//TODO: FIRST GET THE CURRENT DIRECTORY THEN COMPARE WITH THE PATH
	// THEN CHANGE THE WORKING DIRECTORY COMPARE WITH THE RESULTS BEFORE
	//THEN CREATE THE PATH WITH THE MINIMAL STRING OR USE THE OS RETURNED ONE
	current_folder->name_length = strlen(path);
	assert(current_folder->name_length);
	current_folder->folder_name = (char*)calloc(strlen(path) + 1, sizeof(char));
	assert(current_folder->folder_name);
	if (!current_folder->folder_name) goto MOW_FILE_FREE_STRUCT;	//IF WE CAN'T ALLOCATE MEMORY
	current_folder->folder_size += current_folder->name_length;	//INCREASE THE FOLDER SIZE NOT 100% TRUE BUT WILL BE IMPORTANT	
	memcpy(current_folder->folder_name, path, strlen(path));
	do
	{
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))	//MAYBE ALSO CHECK FOR SHORTCUTS AND IGNORE THEM TOO JUST FOR SAME BEHAVIOUR WITH DIFFERENT OPERATING SYSTEMS
		{
			LARGE_INTEGER f_size;
			f_size.LowPart = ffd.nFileSizeLow;
			f_size.HighPart = ffd.nFileSizeHigh;
			uint64_t file_size = (uint64_t)f_size.QuadPart;
			uint64_t name_length = strlen(ffd.cFileName);
			assert(name_length);
			struct mowfile** t = (struct mowfile**)realloc(current_folder->files, (current_folder->file_count + 1) * sizeof(struct mowfile*));
			if (!t) {
				MOW_FILE_ERROR("CAN'T REALLOC");
				goto MOW_FILE_FREE_STRUCT;
			}
			current_folder->file_count++;
			current_folder->files = t;
			struct mowfile* file = (struct mowfile*)calloc(sizeof(struct mowfile), 1);
			if (!file)
				goto MOW_FILE_FREE_STRUCT;    //IF WE CAN'T ALLOCATE MEMORY
			file->file_name = (char*)calloc(name_length + 1, sizeof(char));
			if (!file->file_name) {
				m_free_file(file);
				current_folder->file_count--;
				goto MOW_FILE_FREE_STRUCT;    //IF WE CAN'T ALLOCATE MEMORY
			}
			file->content = (char*)calloc(file_size, sizeof(char));
			if (!file->content) {
				m_free_file(file);
				current_folder->file_count--;
				goto MOW_FILE_FREE_STRUCT;    //IF WE CAN'T ALLOCATE MEMORY
			}
			memcpy(file->file_name, ffd.cFileName, name_length);
			file->name_length = name_length;
			//GET THE FILE CONTENT
			file->content_length = file_size;

#if 0	//WINDOOWS DOUBLE OS DELIMITER VERSION OF THE CODE BUT SEEM WINDOWS HAS NO PROBLEM WITH LINUX OPERATORS
			char t_file_name[MAX_PATH + 2];
			memcpy(t_file_name, path, path_length);
			t_file_name[path_length] = M_OS_DELIMITER_CHAR;
			t_file_name[path_length + 1] = M_OS_DELIMITER_CHAR;
			memcpy(t_file_name + path_length + 2, ffd.cFileName, name_length);
			t_file_name[path_length + name_length + 2] = 0;
#endif
			char t_file_name[MAX_PATH + 1];
			memcpy(t_file_name, path, path_length);
			t_file_name[path_length] = M_OS_DELIMITER_CHAR;
			memcpy(t_file_name + path_length + 1, ffd.cFileName, name_length);
			t_file_name[path_length + name_length + 1] = 0;

			FILE* f = fopen(t_file_name, "rb");
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
		else
		{
			if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0) continue;//MAYBE ALSO CHECK FOR SHORTCUTS AND IGNORE THEM TOO JUST FOR SAME BEHAVIOUR WITH DIFFERENT OPERATING SYSTEMS
			current_folder->folder_count++;

			struct mowfolder** t = (struct mowfolder**)realloc(current_folder->folders, current_folder->folder_count * sizeof(struct mowfolder*));
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
	printf("Error returning\n");
	return NULL;
}
#else
struct mowfolder* m_read_folder(const char* path) {
	if (NULL == path) {
		assert(path);
		return NULL;
	}

	size_t path_length = strlen(path);
	DIR* dir = opendir(path);
	if (!dir) {
		MOW_FILE_ERROR("Can't open folder");
		printf("%s %s\n", strerror(errno), path);
		goto MOW_FILE_RETURN;
	}

	//CREATE THE FOLDER STRUCT
	struct mowfolder* current_folder = calloc(sizeof(struct mowfolder), 1);
	assert(current_folder);
	if (!current_folder) {    //HANDLE IF WE CAN'T ALLOCATE
		goto MOW_FILE_CLOSE_HANDLE;
	}
	//TODO: FIRST GET THE CURRENT DIRECTORY THEN COMPARE WITH THE PATH
	// THEN CHANGE THE WORKING DIRECTORY COMPARE WITH THE RESULTS BEFORE
	//THEN CREATE THE PATH WITH THE MINIMAL STRING OR USE THE OS RETURNED ONE
	current_folder->name_length = strlen(path);
	current_folder->folder_name = calloc(strlen(path) + 1, sizeof(char));
	if (!current_folder->folder_name) goto MOW_FILE_FREE_STRUCT;    //IF WE CAN'T ALLOCATE MEMORY
	current_folder->folder_size += current_folder->name_length;    //INCREASE THE FOLDER SIZE NOT 100% TRUE BUT WILL BE IMPORTANT	
	memcpy(current_folder->folder_name, path, strlen(path));
	struct dirent* ent;
	while ((ent = readdir(dir)) != NULL) {
		if (ent->d_type == DT_REG) {
			//printf("%d is the length of the file %s\n",(int)ent->d_reclen,ent->d_name);
			uint64_t file_size = 0;
			uint64_t name_length = strlen(ent->d_name);
			assert(name_length + path_length + 1 <= M_PATH_MAX);
			char t_file_name[M_PATH_MAX + 1];
			memcpy(t_file_name, path, path_length);
			t_file_name[path_length] = M_OS_DELIMITER_CHAR;
			memcpy(t_file_name + path_length + 1, ent->d_name, name_length);
			t_file_name[path_length + name_length + 1] = 0;
			FILE* f = fopen(t_file_name, "rb");
			if (f) {
				fseeko(f, 0, SEEK_END);
				off_t s = ftello(f);
				if (s == -1) {
					printf("ERROR\n");
				}
				file_size = (uint64_t)s;
				fseeko(f, 0, SEEK_SET);
			}
			else {    //HANDLE CAN'T OPEN FILE FOR READ
				printf("Can't open file %s %s\n", ent->d_name, t_file_name);
				printf("%s\n", strerror(errno));
				continue;
			}
			struct mowfile** t = realloc(current_folder->files, (current_folder->file_count + 1) * sizeof(struct mowfile*));
			if (!t) {
				MOW_FILE_ERROR("CAN'T REALLOC");
				goto MOW_FILE_FREE_STRUCT;
			}
			current_folder->file_count++;
			current_folder->files = t;
			struct mowfile* file = calloc(sizeof(struct mowfile), 1);
			if (!file)
				goto MOW_FILE_FREE_STRUCT;    //IF WE CAN'T ALLOCATE MEMORY
			file->file_name = calloc(name_length + 1, sizeof(char));
			if (!file->file_name) {
				m_free_file(file);
				current_folder->file_count--;
				goto MOW_FILE_FREE_STRUCT;    //IF WE CAN'T ALLOCATE MEMORY
			}
			file->content = calloc(file_size, sizeof(char));
			if (!file->content) {
				m_free_file(file);
				current_folder->file_count--;
				goto MOW_FILE_FREE_STRUCT;    //IF WE CAN'T ALLOCATE MEMORY
			}
			memcpy(file->file_name, ent->d_name, name_length);
			file->name_length = name_length;
			//GET THE FILE CONTENT
			file->content_length = file_size;
			size_t nb = 0;
			while (nb < file_size) {
				size_t tb = fread(file->content + nb, sizeof(char), (size_t)(file_size - nb), f);
				if (!tb) MOW_FILE_ERROR("FREAD");
				nb += tb;
			}
			fclose(f);

			current_folder->files[current_folder->file_count - 1] = file;
			current_folder->folder_size += file->content_length +
				file->name_length;//INCREASE THE FOLDER SIZE NOT 100% TRUE BUT WILL BE IMPORTANT
		}
		else {
			if ((ent->d_type == DT_LNK) || (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)) continue;
			current_folder->folder_count++;

			struct mowfolder** t = realloc(current_folder->folders,
				current_folder->folder_count * sizeof(struct mowfolder*));
			if (!t) {
				MOW_FILE_ERROR("CAN'T REALLOC");
				current_folder->folder_count--;
				goto MOW_FILE_FREE_STRUCT;
			}
			char tmp[M_PATH_MAX];    //CREATE THE TEMPORARY PATH NAME FOR RECURSIVE FUNCTION CALL
			memcpy(tmp, path, path_length);
			tmp[path_length] = M_OS_DELIMITER_CHAR;
			memcpy(tmp + path_length + 1, ent->d_name, strlen(ent->d_name));
			tmp[path_length + 1 + strlen(ent->d_name)] = 0;
			struct mowfolder* child_folder = m_read_folder(tmp);

			if (child_folder) {
				current_folder->folders = t;
				current_folder->folders[current_folder->folder_count - 1] = child_folder;
				child_folder->parrent_folder = current_folder;
				current_folder->folder_size += child_folder->folder_size +
					child_folder->name_length;//INCREASE THE FOLDER SIZE NOT 100% TRUE BUT WILL BE IMPORTANT
			}
			else {
				current_folder->folder_count--;
				goto MOW_FILE_FREE_STRUCT;
			}

		}

	}


	if (closedir(dir)) {
		MOW_FILE_ERROR("Can't close folder handler");
		printf("%s\n", strerror(errno));
	}
	return current_folder;

	//ERROR CONDITIONS RIGHT HERE BEFORE RETURN
MOW_FILE_FREE_STRUCT:
	m_free_folder(current_folder);
MOW_FILE_CLOSE_HANDLE:
	if (closedir(dir)) MOW_FILE_ERROR("Can't close folder handler");
MOW_FILE_RETURN:
	printf("Error returning\n");
	return NULL;
}

#endif

struct mowfile* m_read_file(const char* file_name) {
	if (NULL == file_name) {
		assert(file_name);
		return NULL;
	}

	return NULL;
}

//CHECK IF THE PATH CONTAINS ANY DELIMITER THAT IS NOT OS SPECIFIC
//IF CONTAINS RETURN 1 IF NOT COMPATIBLE RETURN 0
int m_path_compatible(char* path) {    //TODO:(kerem) ADD VECTOR OPERATIONS FOR FASTER CHECK
	if (NULL == path) {
		assert(path);
		return 0;
	}
	size_t string_length = strlen(path);
	//TODO:(kerem) PAD THE BUFFER THAN MAKE VECTOR OPERATIONS FOR FASTER SEARCH
	size_t i;
	for (i = 0; i < string_length; i++) {
		if (path[i] == M_OS_FALSE_DELIMITER_CHAR) return 0;
	}
	return 1;
}

//CONVERT THE OS PATH DELIMITERS
void m_path_conv_compat(char* path) {    //TODO:(kerem) ADD VECTOR OPERATIONS FOR FASTER CHECK
	if (NULL == path) {
		assert(path);
		return;
	}
	size_t string_length = strlen(path);
	//TODO:(kerem) PAD THE BUFFER THAN MAKE VECTOR OPERATIONS FOR FASTER SEARCH
	size_t i;
	for (i = 0; i < string_length; i++) {
		if (path[i] == M_OS_FALSE_DELIMITER_CHAR) path[i] = M_OS_DELIMITER_CHAR;
	}
}


int
m_free_folder(struct mowfolder* folder) {    //TODO:(kerem) CHANGE OR MAYBE ADD A NEW FUNCTION FOR ITERATIVELY FREEING
	assert(folder);
	if (folder) {    //TRAVERSE THE FOLDERS BY ITERATING
		for (uint64_t i = 0; i < folder->file_count; i++) {
			MOW_FILE_CHECK(m_free_file(folder->files[i]));
		}
		if (folder->files)free(folder->files);
		for (uint64_t i = 0; i < folder->folder_count; i++) {
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
	}
	return MOWFILEERR;
}

uint64_t* m_folder_traverse(struct mowfolder* folder) {
	uint64_t* file_folder_c = (uint64_t*)calloc(sizeof(uint64_t), 2);
	assert(file_folder_c);
	if (file_folder_c) {
		for (uint64_t i = 0; i < folder->folder_count; ++i) {
			uint64_t* c_file_folder_c = m_folder_traverse(folder->folders[i]);
#ifdef _WIN32
			printf("<DIR>%s\tSize:%I64u\tFiles:%I64u\tFolders:%I64u\n", folder->folders[i]->folder_name, folder->folders[i]->folder_size, folder->folders[i]->file_count, folder->folders[i]->folder_count);
#else
			printf("<DIR>%s\tSize:%lu\tFiles:%lu\tFolders:%lu\n", folder->folders[i]->folder_name, folder->folders[i]->folder_size, folder->folders[i]->file_count, folder->folders[i]->folder_count);
#endif

			assert(c_file_folder_c);
			if (c_file_folder_c) {
				file_folder_c[0] += c_file_folder_c[0];
				file_folder_c[1] += c_file_folder_c[1];
				free(c_file_folder_c);
			}
			else {
				printf("m_folder_traverse recursion error\n");
				free(file_folder_c);
				return NULL;
			}
		}
		for (uint64_t j = 0; j < folder->file_count; ++j) {
#ifdef _WIN32
			printf("%s\t%I64u\n", folder->files[j]->file_name, folder->files[j]->content_length);
#else
			printf("%s\t%lu\n", folder->files[j]->file_name, folder->files[j]->content_length);
#endif
		}
#ifdef _WIN32
		printf("<DIR>%s\tSize:%I64u\n", folder->folder_name, folder->folder_size);
#else
		printf("<DIR>%s\tSize:%lu\n", folder->folder_name, folder->folder_size);
#endif
		file_folder_c[0] += folder->file_count;
		file_folder_c[1] += folder->folder_count;
	}
	else {
		printf("Can't allocate memory");
		MOW_FILE_ERROR("m_folder_traverse");
	}


	return file_folder_c;
}

void m_folder_print(struct mowfolder* folder) {
	if (folder) {
		uint64_t* file_folder_c = m_folder_traverse(folder);
		if (file_folder_c) {
#ifdef _WIN32
			printf("Files:%I64u Folders:%I64u\n", file_folder_c[0], file_folder_c[1]);
#else
			printf("Files:%lu Folders:%lu\n", file_folder_c[0], file_folder_c[1]);
#endif
			free(file_folder_c);
		}
		else {
			printf("Error\n");
		}


	}
}


static inline char* get_path_name(char* path) {
	assert(path);
	return strrchr(path, M_OS_DELIMITER_CHAR) + 1;
}


int write_file(const char* abs_file_path, struct mowfile* file) {
	//CHECK THE PARAMETERS
	assert(abs_file_path);
	assert(file);
	if (NULL == abs_file_path || NULL == file) return MOWFILEERR;

	size_t abs_path_len = strlen(abs_file_path);
	char abs_file_name[M_PATH_MAX + 1];
	memcpy(abs_file_name, abs_file_path, abs_path_len);
	abs_file_name[abs_path_len] = M_OS_DELIMITER_CHAR;
#ifdef _WIN32
	abs_file_name[abs_path_len + 1] = M_OS_DELIMITER_CHAR;
	memcpy(abs_file_name + abs_path_len + 2, file->file_name, file->name_length);
	abs_file_name[abs_path_len + 2 + file->name_length] = 0;
#else
	memcpy(abs_file_name + abs_path_len + 1, file->file_name, file->name_length);
	abs_file_name[abs_path_len + 1 + file->name_length] = 0;
#endif

	FILE* f = fopen(abs_file_name, "wb");
	if (NULL == f) {
		MOW_FILE_STRERROR();
		return MOWFILEERR;
	}


	uint64_t byte_count = 0;
	while (byte_count < file->content_length) {
		size_t nb = (size_t)fwrite(file->content + byte_count, sizeof(char), (size_t)(file->content_length - byte_count), f);
		assert(nb);
		if (0 == nb) {
			MOW_FILE_STRERROR();
			fclose(f);
			return MOWFILEERR;
		}
		byte_count += (uint64_t)nb;
	}
	if (f)
		fclose(f);

	return MOWFILEOK;
}

int write_folder(const char* abs_path, struct mowfolder* folder) {
	char tmpPath[M_PATH_MAX + 1];

	for (uint64_t i = 0; i < folder->folder_count; i++)
	{
		size_t path_len = strlen(abs_path);
		memcpy(tmpPath, abs_path, path_len);

#if 0	//WINDOWS DOUBLE PATH DELIMITER VERSION BUT WINDOWS SEEMS TO HAVE NO PROBLEM WITH LINUX PATH DELIMITERS
#ifdef _WIN32
		tmpPath[path_len] = M_OS_DELIMITER_CHAR;
		tmpPath[path_len + 1] = M_OS_DELIMITER_CHAR;
		char* t_folder_name = get_path_name(folder->folders[i]->folder_name);
		size_t t_folder_n_len = strlen(t_folder_name);
		memcpy(tmpPath + path_len + 2, t_folder_name, t_folder_n_len);
		tmpPath[path_len + 2 + t_folder_n_len] = 0;
#endif
#endif

		tmpPath[path_len] = M_OS_DELIMITER_CHAR;
		char* t_folder_name = get_path_name(folder->folders[i]->folder_name);
		size_t t_folder_n_len = strlen(t_folder_name);
		memcpy(tmpPath + path_len + 1, t_folder_name, t_folder_n_len);
		tmpPath[path_len + 1 + t_folder_n_len] = 0;

		if (MOWFILEERR == m_write_folder(tmpPath, folder->folders[i])) return MOWFILEERR;
	}
	for (uint64_t i = 0; i < folder->file_count; i++)
	{
		if (MOWFILEERR == write_file(abs_path, folder->files[i])) return MOWFILEERR;
	}
	return MOWFILEOK;
}

int m_write_folder(char* abs_path, struct mowfolder* folder) {
	assert(folder);
	assert(abs_path);
	if (!folder || !abs_path || abs_path[0] == '.' || abs_path[0] == '~') {
		return 0;
	}
	if (MOWFILEERR == m_dir_exist(abs_path)) {
		if (MOWFILEERR == m_create_dir(abs_path)) return MOWFILEERR;
	}
	if (MOWFILEERR == write_folder(abs_path, folder)) return MOWFILEERR;
	return MOWFILEOK;
}

char* m_get_current_dir(void) {
	char* ret = NULL;
#ifdef _WIN32
	DWORD len = GetCurrentDirectoryA(0, NULL);
	ret = calloc(((size_t)len) + 1, sizeof(char));
	assert(ret);
	if (!ret) return NULL;
	if (len < GetCurrentDirectoryA(len, ret)) {
		free(ret);
		printf("Directory changed?\n");
		return NULL;
	}
#else
	ret = calloc(M_PATH_MAX + 1, sizeof(char));
	assert(ret);
	if (!ret) return NULL;
	if (NULL == getcwd(ret, M_PATH_MAX)) {
		free(ret);
		return NULL;
	}
#endif
	return ret;
}

int m_set_current_dir(const char* abs_path) {

	return MOWFILEERR;
}

int m_dir_exist(const char* abs_path) {
	assert(abs_path);
	if (NULL == abs_path) return MOWFILEERR;
	int dir_exist = 0;
#ifdef _WIN32
	dir_exist = _access(abs_path, 0);
#else
	dir_exist = access(abs_path, 0);
#endif
	if (0 != dir_exist) {	//DIRECTORY DOES NOT EXIST
		return MOWFILEERR;
	}
	return MOWFILEOK;
}

int create_dir(const char* abs_path) {
	assert(abs_path);
	if (NULL == abs_path || MOWFILEOK == m_dir_exist(abs_path)) return MOWFILEERR;	//IF EXIST RETURN ERROR
#ifdef _WIN32
	if (0 != _mkdir(abs_path)) {
#else
	if (0 != mkdir(abs_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
#endif
		MOW_FILE_ERR_PARAM_STR(MOWFILEERR, abs_path);
		return MOWFILEERR;
	}
	return MOWFILEOK;
}

int m_create_dir(char* abs_path) {
	assert(abs_path);
	if (NULL == abs_path || abs_path[0] != '/' || MOWFILEOK == m_dir_exist(abs_path)) return MOWFILEERR;

	char* path_pos = abs_path + 1;
	uint64_t pos = 0;
	char* t_path = NULL;
	while (path_pos) {
		path_pos = strchr(path_pos, M_OS_DELIMITER_CHAR);
		if (path_pos) {
			pos = path_pos - abs_path;
			t_path = calloc(pos + 1, sizeof(char));
			assert(t_path);
			if (NULL == t_path) {
				MOW_FILE_ERROR("Can't allocate memory\n");
				return MOWFILEERR;
			}
			memcpy(t_path, abs_path, pos);
#if 0
			abs_path[pos] = 0;
			create_dir(abs_path);
			abs_path[pos] = M_OS_DELIMITER_CHAR;
#endif
			create_dir(t_path);
			path_pos++;
			free(t_path);
		}
	}
	if(MOWFILEERR == create_dir(abs_path)) return MOWFILEERR;
	return MOWFILEOK;
}