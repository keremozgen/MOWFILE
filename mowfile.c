#include "mowfile.h"



//TODO:(kerem) MAYBE CHANGE THIS
#ifdef __ANDROID__
void android_main() {
#else
void main(void) {
#endif
#if defined(_WIN32) && defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif


#ifdef _WIN32
    uint64_t size = 128;
	char* path = malloc(size);
	if (!path) return;
	for (uint64_t i = 0; i < size - 2; i++)
	{
		path[i] = M_OS_FALSE_DELIMITER_CHAR;
	}

	path[size - 2] = M_OS_DELIMITER_CHAR;
	path[size - 1] = 0;

	int r = m_path_compatible(path);
	printf("%d returned\n",r);
	m_path_conv_compat(path);
	printf("%s", path);
	free(path);

    struct mowfolder* tmp = m_read_folder("D:\\projects\\mowfile");
    m_folder_print(tmp);
    m_free_folder(tmp);

#else

    struct mowfolder* tmp = m_read_folder("/home/asus/Desktop/PROJECT/");
    //m_folder_print(tmp);
    m_free_folder(tmp);

#endif

#if defined(_WIN32) && defined(_DEBUG)
	_CrtDumpMemoryLeaks();
#endif
}



