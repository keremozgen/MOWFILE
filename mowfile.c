
#define M_LIN_DELIMITER 
#include "mowfile.h"

//TODO:(kerem) MAYBE CHANGE THIS
#ifdef __ANDROID__
void android_main(struct android_app* app) {
#else
int main(void) {
#endif
#if defined(_WIN32) && defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif
	char * cwd = m_get_current_dir();
	if (cwd) printf("%s\n", cwd);
	free(cwd);

#ifdef _WIN32
	struct mowfolder* tmp = m_read_folder("D:/projects/mowfile");
#elif __ANDROID__
	printf("%s is the android current working directory\n", getcwd(NULL, 0));
	struct mowfolder* tmp = m_read_folder("/data/data/org.fips.mowfile/files");
#elif __linux__
	struct mowfolder* tmp = m_read_folder("/home/asus/Desktop/PROJECT/MOWFILE");
	if (!tmp) { printf("WSL\n"); tmp = m_read_folder("/mnt/d/PROJECTS/MOWFILE/"); }
#endif
	if (tmp) {
		//m_folder_print(tmp);
		m_write_folder("/backupFOlder",tmp);
		m_free_folder(tmp);
	}
	
	printf("\nExit!\n");
#if defined(_WIN32) && defined(_DEBUG)
	_CrtDumpMemoryLeaks();
#endif
	return 0;
}



