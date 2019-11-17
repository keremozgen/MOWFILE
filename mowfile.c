#include "mowfile.h"

//TODO:(kerem) MAYBE CHANGE THIS
#ifdef __ANDROID__
void android_main(struct android_app* app) {
#else
void main(void) {
#endif
#if defined(_WIN32) && defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif


#ifdef _WIN32
	struct mowfolder* tmp = m_read_folder("D:\\projects\\mowfile");
#elif __ANDROID__
	printf("%s is the android current working directory\n", getcwd(NULL, 0));
	struct mowfolder* tmp = m_read_folder("/data/data/org.fips.mowfile/files");
#elif __linux__
	struct mowfolder* tmp = m_read_folder("/home/asus/Desktop/PROJECT/MOWFILE");
#endif
	if (tmp) {
		m_folder_print(tmp);
		m_free_folder(tmp);
	}
	
#if defined(_WIN32) && defined(_DEBUG)
	_CrtDumpMemoryLeaks();
#endif
}



