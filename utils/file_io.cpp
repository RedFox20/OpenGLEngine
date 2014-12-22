#include "file_io.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>




	static void* OpenFile(const char* filename, IOFlags mode, bool noBuffering)
	{
		int desiredAccess;
		int sharingMode;			// FILE_SHARE_READ, FILE_SHARE_WRITE
		int creationDisposition;	// OPEN_EXISTING, OPEN_ALWAYS, CREATE_ALWAYS
		int openFlags;
		switch (mode)
		{
			default:
			case READONLY:
				desiredAccess       = FILE_GENERIC_READ;
				sharingMode			= FILE_SHARE_READ | FILE_SHARE_WRITE;
				creationDisposition = OPEN_EXISTING;
				openFlags			= noBuffering ? FILE_FLAG_NO_BUFFERING : FILE_ATTRIBUTE_NORMAL;
				break;
			case READONLYEXECUTE:
				desiredAccess       = FILE_GENERIC_READ|FILE_GENERIC_EXECUTE;
				sharingMode			= FILE_SHARE_READ | FILE_SHARE_WRITE;
				creationDisposition = OPEN_EXISTING;
				openFlags			= noBuffering ? FILE_FLAG_NO_BUFFERING : FILE_ATTRIBUTE_NORMAL;
				break;
			case READWRITE:
				desiredAccess       = FILE_GENERIC_READ|FILE_GENERIC_WRITE;
				sharingMode			= FILE_SHARE_READ;
				creationDisposition = OPEN_EXISTING; // if not exists, fail
				openFlags           = FILE_ATTRIBUTE_NORMAL;
				break;
			case READWRITECREATE:
				desiredAccess       = FILE_GENERIC_READ|FILE_GENERIC_WRITE;
				sharingMode			= FILE_SHARE_READ;
				creationDisposition = OPEN_ALWAYS; // if exists, success, else create file
				openFlags           = FILE_ATTRIBUTE_NORMAL;
				break;
			case CREATENEW:
				desiredAccess       = FILE_GENERIC_READ|FILE_GENERIC_WRITE|DELETE;
				sharingMode			= FILE_SHARE_READ;
				creationDisposition = CREATE_ALWAYS;
				openFlags           = FILE_ATTRIBUTE_NORMAL;
				break;
			case CREATETEMP:
				desiredAccess		= FILE_GENERIC_READ|FILE_GENERIC_WRITE|DELETE;
				sharingMode			= FILE_SHARE_DELETE|FILE_SHARE_READ;
				creationDisposition = CREATE_ALWAYS;
				openFlags			= FILE_FLAG_DELETE_ON_CLOSE | FILE_ATTRIBUTE_TEMPORARY;
				break;
		}

		SECURITY_ATTRIBUTES secu = { sizeof(secu), NULL, TRUE };
		void* handle = CreateFileA(filename, desiredAccess, sharingMode, &secu, creationDisposition, openFlags, 0);
		return handle != INVALID_HANDLE_VALUE ? handle : 0;
	}








	unbuffered_file::unbuffered_file(const char* filename, IOFlags mode) 
		: Handle(OpenFile(filename, mode, true)), Mode(mode)
	{
	}
	unbuffered_file::unbuffered_file(const std::string& filename, IOFlags mode)
		: Handle(OpenFile(filename.c_str(), mode, true)), Mode(mode)
	{
	}
	unbuffered_file::unbuffered_file(unbuffered_file&& f)
		: Handle(f.Handle), Mode(f.Mode)
	{
		f.Handle = 0;
	}
	unbuffered_file::~unbuffered_file()
	{
		if (Handle)
			CloseHandle(Handle);
	}
	unbuffered_file& unbuffered_file::operator=(unbuffered_file&& f)
	{
		if (Handle)
			CloseHandle(Handle);
		Handle = f.Handle;
		Mode   = f.Mode;
		f.Handle = 0;
		return *this;
	}
	bool unbuffered_file::open(const char* filename, IOFlags mode)
	{
		if (Handle)
			CloseHandle(Handle);
		Mode = mode;
		return (Handle = OpenFile(filename, mode, true)) != NULL;
	}
	void unbuffered_file::close()
	{
		if (Handle)
		{
			CloseHandle(Handle);
			Handle = NULL;
		}
	}
	bool unbuffered_file::good() const
	{
		return Handle != NULL;
	}
	bool unbuffered_file::bad() const
	{
		return Handle == NULL;
	}
	int unbuffered_file::size() const
	{
		return GetFileSize(Handle, NULL);
	}
	unsigned __int64 unbuffered_file::sizel() const
	{
		LARGE_INTEGER fsize = { 0 };
		GetFileSizeEx(Handle, &fsize);
		return fsize.QuadPart;
	}
	int unbuffered_file::size_aligned() const
	{
		int size = GetFileSize(Handle, NULL);
		if (int rem = size % ALIGNMENT)
			return (size - rem) + ALIGNMENT;
		return size; // already aligned
	}
	int unbuffered_file::read(void* buffer, int bytesToRead)
	{
		if (bytesToRead % ALIGNMENT)
			throw "unbuffered_file::read() 'bytesToRead' must always be aligned to 4KB!";
		DWORD bytesRead;
		ReadFile(Handle, buffer, bytesToRead, &bytesRead, NULL);
		return (int)bytesRead;
	}
	load_buffer unbuffered_file::readAll()
	{
		int alignedSize = GetFileSize(Handle, NULL);
		if (!alignedSize)
			return load_buffer(0, 0);

		if (int rem = alignedSize % ALIGNMENT)
			alignedSize = (alignedSize - rem) + ALIGNMENT;

		char* buffer = (char*)malloc(alignedSize);

		DWORD bytesRead;
		ReadFile(Handle, buffer, alignedSize, &bytesRead, NULL);
		return load_buffer(buffer, bytesRead);
	}
	load_buffer unbuffered_file::readAll(const char* filename)
	{
		return unbuffered_file(filename, READONLY).readAll();
	}
	load_buffer unbuffered_file::readAll(const token& filename)
	{
		char fileName[512];
		int len = filename.length();
		memcpy(fileName, filename.c_str(), len);
		fileName[len] = '\0';
		return readAll(fileName);
	}
	int unbuffered_file::write(const void* buffer, int bytesToWrite)
	{
		DWORD bytesWritten;
		WriteFile(Handle, buffer, bytesToWrite, &bytesWritten, NULL);
		return (int)bytesWritten;
	}
	int unbuffered_file::writenew(const char* filename, const void* buffer, int bytesToWrite)
	{
		return unbuffered_file(filename, IOFlags::CREATENEW).write(buffer, bytesToWrite);
	}
	int unbuffered_file::seek(int filepos, int seekmode)
	{
		return SetFilePointer(Handle, filepos, NULL, seekmode);
	}
	int unbuffered_file::tell() const
	{
		return SetFilePointer(Handle, 0, NULL, FILE_CURRENT);
	}
	unsigned __int64 unbuffered_file::time_created() const
	{
		FILETIME ft = { 0 };
		GetFileTime(Handle, &ft, 0, 0);
		return *(unsigned __int64*)&ft;
	}
	unsigned __int64 unbuffered_file::time_accessed() const
	{
		FILETIME ft = { 0 };
		GetFileTime(Handle, 0, &ft, 0);
		return *(unsigned __int64*)&ft;
	}
	unsigned __int64 unbuffered_file::time_modified() const
	{
		FILETIME ft = { 0 };
		GetFileTime(Handle, 0, 0, &ft);
		return *(unsigned __int64*)&ft;
	}









	file::file(const char* filename, IOFlags mode) 
		: Handle(OpenFile(filename, mode, false)), Mode(mode)
	{
	}
	file::file(const std::string& filename, IOFlags mode)
		: Handle(OpenFile(filename.c_str(), mode, false)), Mode(mode)
	{
	}
	file::file(file&& f)
		: Handle(f.Handle), Mode(f.Mode)
	{
		f.Handle = 0;
	}
	file::~file()
	{
		if (Handle)
			CloseHandle(Handle);
	}
	file& file::operator=(file&& f)
	{
		if (Handle)
			CloseHandle(Handle);
		Handle = f.Handle;
		Mode   = f.Mode;
		f.Handle = 0;
		return *this;
	}
	bool file::open(const char* filename, IOFlags mode)
	{
		if (Handle)
			CloseHandle(Handle);
		Mode = mode;
		return (Handle = OpenFile(filename, mode, false)) != NULL;
	}
	void file::close()
	{
		if (Handle)
		{
			CloseHandle(Handle);
			Handle = NULL;
		}
	}
	bool file::good() const
	{
		return Handle != NULL;
	}
	bool file::bad() const
	{
		return Handle == NULL;
	}
	int file::size() const
	{
		return GetFileSize(Handle, NULL);
	}
	unsigned __int64 file::sizel() const
	{
		LARGE_INTEGER fsize = { 0 };
		GetFileSizeEx(Handle, &fsize);
		return fsize.QuadPart;
	}
	int file::read(void* buffer, int bytesToRead)
	{
		DWORD bytesRead;
		ReadFile(Handle, buffer, bytesToRead, &bytesRead, NULL);
		return (int)bytesRead;
	}
	load_buffer file::readAll()
	{
		int fileSize = GetFileSize(Handle, NULL);
		if (!fileSize)
			return load_buffer(0, 0);

		char* buffer = (char*)malloc(fileSize);
		DWORD bytesRead;
		ReadFile(Handle, buffer, fileSize, &bytesRead, NULL);
		return load_buffer(buffer, bytesRead);
	}
	int file::write(const void* buffer, int bytesToWrite)
	{
		DWORD bytesWritten;
		WriteFile(Handle, buffer, bytesToWrite, &bytesWritten, NULL);
		return (int)bytesWritten;
	}
	int file::writenew(const char* filename, const void* buffer, int bytesToWrite)
	{
		return unbuffered_file(filename, IOFlags::CREATENEW).write(buffer, bytesToWrite);
	}
	int file::seek(int filepos, int seekmode)
	{
		return SetFilePointer(Handle, filepos, NULL, seekmode);
	}
	int file::tell() const
	{
		return SetFilePointer(Handle, 0, NULL, FILE_CURRENT);
	}
	unsigned __int64 file::time_created() const
	{
		FILETIME ft = { 0 };
		GetFileTime(Handle, &ft, 0, 0);
		return *(unsigned __int64*)&ft;
	}
	unsigned __int64 file::time_accessed() const
	{
		FILETIME ft = { 0 };
		GetFileTime(Handle, 0, &ft, 0);
		return *(unsigned __int64*)&ft;
	}
	unsigned __int64 file::time_modified() const
	{
		FILETIME ft = { 0 };
		GetFileTime(Handle, 0, 0, &ft);
		return *(unsigned __int64*)&ft;
	}









	bool file_exists(const char* file)
	{
		DWORD attr = GetFileAttributesA(file);
		return attr != -1 && (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
	}
	bool file_exists(const std::string& file)
	{
		DWORD attr = GetFileAttributesA(file.c_str());
		return attr != -1 && (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
	}
	bool file_exists(const token& tokStr)
	{
		char buffer[MAX_PATH];
		int len = tokStr.length();
		memcpy(buffer, tokStr.str, len);
		buffer[len] = 0;
		DWORD attr = GetFileAttributesA(buffer);
		return attr != -1 && (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
	}


	bool folder_exists(const char* folder)
	{
		DWORD attr = GetFileAttributesA(folder);
		return attr != -1 && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
	bool folder_exists(const std::string& folder)
	{
		DWORD attr = GetFileAttributesA(folder.c_str());
		return attr != -1 && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
	bool folder_exists(const token& folder)
	{
		char buffer[MAX_PATH];
		int len = int(folder.end - folder.str);
		memcpy(buffer, folder.str, len);
		buffer[len] = 0;
		DWORD attr = GetFileAttributesA(buffer);
		return attr != -1 && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}

	size_t file_size(const char* file)
	{
		WIN32_FILE_ATTRIBUTE_DATA fad;
		if (!GetFileAttributesExA(file, GetFileExInfoStandard, &fad))
			return -1;
		return fad.nFileSizeLow;
	}
	size_t file_size(const std::string& file)
	{
		WIN32_FILE_ATTRIBUTE_DATA fad;
		if (!GetFileAttributesExA(file.c_str(), GetFileExInfoStandard, &fad))
			return -1;
		return fad.nFileSizeLow;
	}

	time_t file_modified(const char* file)
	{
		return unbuffered_file(file).time_modified();
	}
	time_t file_modified(const std::string& file)
	{
		return unbuffered_file(file).time_modified();
	}


	int directory::list_dirs(std::vector<std::string>& out, const char* directory, const char* matchPattern)
	{
		out.clear();

		char findPattern[MAX_PATH * 2];
		sprintf(findPattern, "%s\\%s", directory, matchPattern);

		WIN32_FIND_DATAA ffd;
		HANDLE hFind = FindFirstFileA(findPattern, &ffd);
		if (hFind == INVALID_HANDLE_VALUE)
			return 0;

		do
		{
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (strcmp(ffd.cFileName, ".") != 0 && strcmp(ffd.cFileName, "..") != 0)
					out.emplace_back(ffd.cFileName);
			}
		} while (FindNextFileA(hFind, &ffd));
		
		FindClose(hFind);
		return (int)out.size();
	}


	int directory::list_files(std::vector<std::string>& out, const char* directory, const char* matchPattern)
	{
		out.clear();

		char findPattern[MAX_PATH * 2];
		sprintf(findPattern, "%s\\%s", directory, matchPattern);

		WIN32_FIND_DATAA ffd;
		HANDLE hFind = FindFirstFileA(findPattern, &ffd);
		if (hFind == INVALID_HANDLE_VALUE)
			return 0;

		do
		{
			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				out.emplace_back(ffd.cFileName);
		} while (FindNextFileA(hFind, &ffd));
		
		FindClose(hFind);
		return out.size();
	}


	std::string directory::get_working_dir()
	{
		char path[MAX_PATH * 2];
		return std::string(path, GetCurrentDirectoryA(sizeof path, path));
	}

	void directory::set_working_dir(const std::string& new_wd)
	{
		SetCurrentDirectoryA(new_wd.c_str());
	}

	std::string directory::fullpath(const std::string& relativePath)
	{
		char path[MAX_PATH * 2];
		return std::string(path, GetFullPathNameA(relativePath.c_str(), sizeof path, path, NULL));
	}

	std::string directory::fullpath(const char* relativePath)
	{
		char path[MAX_PATH * 2];
		return std::string(path, GetFullPathNameA(relativePath, sizeof path, path, NULL));
	}

	std::string directory::filename(const std::string& someFilePath)
	{
		char path[MAX_PATH * 2];
		char* file;
		int len = GetFullPathNameA(someFilePath.c_str(), sizeof path, path, &file);
		return std::string(file, (path + len) - file);
	}

	std::string directory::filename(const char* someFilePath)
	{
		char path[MAX_PATH * 2];
		char* file;
		int len = GetFullPathNameA(someFilePath, sizeof path, path, &file);
		return std::string(file, (path + len) - file);
	}

	std::string directory::foldername(const std::string& someFolderPath)
	{
		char path[MAX_PATH * 2];
		char* file;
		GetFullPathNameA(someFolderPath.c_str(), sizeof path, path, &file);
		return std::string(path, file - path);
	}

	std::string directory::foldername(const char* someFolderPath)
	{
		char path[MAX_PATH * 2];
		char* file;
		GetFullPathNameA(someFolderPath, sizeof path, path, &file);
		return std::string(path, file - path);
	}








	dirwatch::dirwatch(const char* folder, dirwatch_flags flags, bool monitorSubDirs) : Handle(0)
	{
		initialize(folder, flags, monitorSubDirs);
	}

	dirwatch::dirwatch(const std::string& folder, dirwatch_flags flags, bool monitorSubDirs) : Handle(0)
	{
		initialize(folder, flags, monitorSubDirs);
	}

	dirwatch::~dirwatch()
	{
		close();
	}

	void dirwatch::close()
	{
		if (Handle)
		{
			FindCloseChangeNotification(Handle);
			Handle = NULL;
		}
	}

	void dirwatch::initialize(const char* folder, dirwatch_flags flags, bool monitorSubDirs)
	{
		if (Handle) FindCloseChangeNotification(Handle);
		Handle = FindFirstChangeNotificationA(folder, monitorSubDirs, flags);
		if (Handle == INVALID_HANDLE_VALUE) Handle = NULL;
	}
	
	void dirwatch::initialize(const std::string& folder, dirwatch_flags flags, bool monitorSubDirs)
	{
		if (Handle) FindCloseChangeNotification(Handle);
		Handle = FindFirstChangeNotificationA(folder.c_str(), monitorSubDirs, flags);
		if (Handle == INVALID_HANDLE_VALUE) Handle = NULL;
	}

	bool dirwatch::wait(int timeoutMillis) const
	{
		DWORD result = WaitForSingleObject(Handle, timeoutMillis);
		FindNextChangeNotification(Handle);
		return result == WAIT_OBJECT_0;
	}

	bool dirwatch::changed() const
	{
		DWORD result = WaitForSingleObject(Handle, 0);
		FindNextChangeNotification(Handle);
		return result == WAIT_OBJECT_0;
	}




