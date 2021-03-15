#define WIN32_LEAN_AND_MEAN
#define NULL 0

#include <Windows.h>

#include <include/oslib/platform.h>

#include <stdio.h>

i32 OSLIB_GetFileSize(const char *filepath)
{
	HANDLE file = CreateFileA(filepath, GENERIC_READ, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_READONLY, NULL);

	DWORD error = GetLastError();

	if (file == INVALID_HANDLE_VALUE)
		return -1;

	DWORD fileSize = GetFileSize(file, NULL);

	error = GetLastError();

	if (file == NULL)
		return -1;

	CloseHandle(file);

	return fileSize;
}

i32 OSLIB_GetDirectoryFileCount(const char *filepath)
{
	size_t filepathLength = strlen(filepath);

	if (filepathLength > (MAX_PATH - 3))
		return -1;

	char *newFilepath = Allocate(sizeof(char) * MAX_PATH);
	memcpy(newFilepath, filepath, sizeof(char) * filepathLength);
	newFilepath[filepathLength] = (char)'\\*';
	newFilepath[filepathLength+1] = (char)'\0';


	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(newFilepath, &fd);

	i32 fileCount = 0;
	do
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}
		else
		{
			fileCount++;
		}
	} while (FindNextFile(hFind, &fd) != 0);

	return fileCount;
}

i32 OSLIB_GetDirectoryFileCountWithExtension(const char *filepath, const char *extension)
{
	size_t filepathLength = strlen(filepath);

	if (filepathLength > (MAX_PATH - 3))
		return -1;

	char *newFilepath = Allocate(sizeof(char) * MAX_PATH);
	memcpy(newFilepath, filepath, sizeof(char) * filepathLength);
	newFilepath[filepathLength] = (char)'\\*';
	newFilepath[filepathLength + 1] = (char)'\0';


	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(newFilepath, &fd);
	size_t extensionLength = strlen(extension);

	i32 fileCount = 0;
	do
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}
		else
		{
			size_t filenameLength = strlen(fd.cFileName);

			if (!strcmp(&fd.cFileName[filenameLength - extensionLength], extension))
				fileCount++;
			
		}
	} while (FindNextFile(hFind, &fd) != 0);

	return fileCount;
}

const char ** OSLIB_GetFilesWithExtensionInDirectory(const char *filepath, const char *extension)
{
	size_t filepathLength = strlen(filepath);

	if (filepathLength > (MAX_PATH - 3))
		return NULL;

	i32 filesWithExtension = OSLIB_GetDirectoryFileCountWithExtension(filepath, extension);

	char **filenames = Allocate(sizeof(char*) * filesWithExtension);

	char *newFilepath = Allocate(sizeof(char) * MAX_PATH);
	memcpy(newFilepath, filepath, sizeof(char) * filepathLength);
	newFilepath[filepathLength] = (char)'\\*';
	newFilepath[filepathLength + 1] = (char)'\0';


	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(newFilepath, &fd);
	size_t extensionLength = strlen(extension);

	i32 fileCount = 0;

	do
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}
		else
		{
			size_t filenameLength = strlen(fd.cFileName);

			if (!strcmp(&fd.cFileName[filenameLength - extensionLength], extension))
			{
				char *filepathString = Allocate(sizeof(char) * (filepathLength + filenameLength) + 1);
				memcpy(filepathString, filepath, sizeof(char) * filepathLength);
				memcpy(&filepathString[filepathLength], fd.cFileName, sizeof(char) * filenameLength);
				filepathString[filepathLength + filenameLength] = '\0';
				filenames[fileCount++] = filepathString;
			}

		}
	} while (FindNextFile(hFind, &fd) != 0);

	return filenames;
}

i32 OSLIB_ReadBytesFromFile(const char *filepath, i8 *buffer, i32 bufferSize)
{
	HANDLE file = CreateFileA(filepath, GENERIC_READ, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_READONLY, NULL);

	DWORD error = GetLastError();

	DWORD fileSize = GetFileSize(file, NULL);

	if (file == NULL)
		return -1;

	if (buffer == NULL)
	{
		CloseHandle(file);
		return fileSize;
	}

	DWORD bytes;

	ReadFile(file, buffer, bufferSize, &bytes, NULL);
	CloseHandle(file);


	return fileSize;
}

i32 OSLIB_WriteBytesToFile(const char *filepath, i8 *buffer, i32 bufferSize)
{
	HANDLE file = CreateFileA(filepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	DWORD error = GetLastError();

	if (file == INVALID_HANDLE_VALUE)
		file = CreateFileA(filepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

	error = GetLastError();

	u32 bytesWritten;
	WriteFile(file, buffer, bufferSize, &bytesWritten, NULL);
	error = GetLastError();

	CloseHandle(file);

	return bytesWritten;
}

i32 OSLIB_AppendBytesToFile(const char *filepath, i8 *buffer, i32 bufferSize)
{
	HANDLE file = CreateFileA(filepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	DWORD error = GetLastError();

	if (file == INVALID_HANDLE_VALUE)
		file = CreateFileA(filepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

	error = GetLastError();

	u32 bytesWritten;
	SetFilePointer(file, 0, NULL, FILE_END);
	WriteFile(file, buffer, bufferSize, &bytesWritten, NULL);
	error = GetLastError();

	CloseHandle(file);

	return bytesWritten;
}