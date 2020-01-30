# MOWFILE
MOW FILE is a cross platform, easy to use and a basic file read-write library.
	
	It includes basic file/folder i/o operations. Here is a list of functions and explanations for them:
	
	int m_create_dir(char* abs_path)	Gets a path and creates the directory. This is recursive so if needed creates multiple directories.
	int m_dir_exist(const char* abs_path)	Is this directory exists.
	char* m_get_current_dir(void)			Get the current working directory.
	int m_set_current_dir(const char* abs_path)	Set the current working directory. Be careful when used in threads.
	
	struct mowfolder* m_read_folder(const char* path);	Read all the directories including sub-directories and files too.

	struct mowfile* m_read_file(const char* file_name);	Read the file if exists.

	int m_path_compatible(char* path);	Path delimiters are different on windows and linux. Checks if the delimiters are rigth

	void m_path_conv_compat(char* path);	Convert delimiters to OS delimiter.

	int m_free_folder(struct mowfolder* folder);	Don't forget to free the folder struct that has been created by m_read_folder

	int m_free_file(struct mowfile* file);	Don't forget to free the file struct that has been created by m_read_file

	void m_folder_print(struct mowfolder* folder);	You can use this function to print folder struct

	int m_write_folder(char* abs_path, struct mowfolder* folder);	Write the folder struct.

	int m_write_file(const char* abs_file_path, struct mowfile* file);	Write the file struct.





