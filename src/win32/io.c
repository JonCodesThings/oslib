#define WIN32_LEAN_AND_MEAN
#define NULL 0

#include <Windows.h>

#include <include/oslib/platform.h>

#include <stdio.h>

static enum FSQueryFlags
{
	FSQueryFlags_None = 0,
	FSQueryFlags_FileCount = 1 << 0,
	FSQueryFlags_DirectoryCount = 1 << 1,
	FSQueryFlags_FileNames = 1 << 2,
	FSQueryFlags_DirectoryNames = 1 << 3,
};

static enum FSTypeFlag
{
	FSTypeFlag_None = 0,
	FSTypeFlag_File = 1 << 0,
	FSTypeFlag_Directory = 1 << 1
};

typedef struct FSQuery
{
	enum FSQueryFlags flags;
	size_t pathLength;
	size_t extensionLength;
	const char *path;
	const char *extension;
} FSQuery;

typedef struct FSQueryData
{
	size_t nameLength;
	const char *name;
	struct FSQueryData *next;
} FSQueryData;

typedef struct FSQueryResults
{
	u32 DirectoryCount;
	u32 FileCount;
	struct FSQueryData *Directories;
	struct FSQueryData *MatchingFiles;
} FSQueryResults;

static void InitializeQuery(FSQuery *query, const enum FSQueryFlags flags, const char *path, size_t pathLength, const char *extension, size_t extensionLength)
{
	query->path = path;
	query->pathLength = pathLength;
	query->flags = flags;
	query->extension = extension;
	query->extensionLength = extensionLength;
}

static void InitializeQueryResults(FSQueryResults *results)
{
	results->Directories = NULL;
	results->DirectoryCount = 0;
	results->FileCount = 0;
	results->MatchingFiles = NULL;
}

static const char* AllocateNamestring(const FSQuery* query, const char *fdName, const size_t fdNameLength, const enum FSTypeFlag flags)
{
	u32 finalNamestringLength = query->pathLength + fdNameLength;
	// If it's a directory we're going to add a trailing '/'
	if (flags & FSTypeFlag_Directory)
	{
		finalNamestringLength++;
	}
	// NULL terminator
	finalNamestringLength++;

	char* str = Allocate(sizeof(char) * finalNamestringLength);
	if (!str)
	{
		return NULL;
	}

	memcpy(str, query->path, sizeof(char) * query->pathLength);
	memcpy(&str[query->pathLength], fdName, sizeof(char) * fdNameLength);
	if (flags & FSTypeFlag_Directory)
	{
		str[query->pathLength + fdNameLength] = '/';
	}
	str[finalNamestringLength - 1] = '\0';
	return str;
}

static void InitializeQueryData(FSQueryData** data, const char* name, const size_t nameLength)
{
	*data = Allocate(sizeof(FSQueryData));
	(*data)->name = name;
	(*data)->nameLength = nameLength;
	(*data)->next = NULL;
}

static void AppendQueryData(FSQueryData* const data, const char *name, const size_t nameLength)
{
	FSQueryData* entry = data;
	while (entry->next != NULL)
	{
		entry = entry->next;
	}
	FSQueryData *next = Allocate(sizeof(FSQueryData));
	next->name = name;
	next->nameLength = nameLength;
	next->next = NULL;
	entry->next = next;
}

static void UpdateQueryResults(const FSQuery* query, FSQueryData** results, const char* name, const size_t nameLength, const enum FSTypeFlags type)
{
	const char* namestring = AllocateNamestring(query, name, nameLength, type);
	if (!namestring)
	{
		return;
	}

	const size_t namestringLength = strlen(namestring);

	if (!*results)
	{
		InitializeQueryData(results, namestring, namestringLength);
	}
	else
	{
		AppendQueryData(*results, namestring, namestringLength);
	}
}

static void QueryFileSystem(struct FSQuery *query, FSQueryResults *const results)
{
	if (!query || !results)
	{
		return;
	}

	char* newPath = Allocate(sizeof(char) * MAX_PATH);
	if (!newPath)
	{
		return;
	}

	memcpy(newPath, query->path, sizeof(char) * query->pathLength);
	newPath[query->pathLength] = (char)'\\*';
	newPath[query->pathLength + 1] = (char)'\0';

	WIN32_FIND_DATAA fd;
	HANDLE hFind = FindFirstFileA(newPath, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		Deallocate(newPath);
		return;
	}

	do
	{
		size_t fdNameLength = strlen(fd.cFileName);

		if (fd.cFileName && fd.cFileName[0] == '.')
		{
			if (fdNameLength == 1)
				continue;

			if (fdNameLength == 2 && fd.cFileName[1] == '.')
				continue;
		}

		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (query->flags & FSQueryFlags_DirectoryCount)
			{
				results->DirectoryCount++;
			}

			if (query->flags & FSQueryFlags_DirectoryNames)
			{
				UpdateQueryResults(query, &results->Directories, fd.cFileName, fdNameLength, FSTypeFlag_Directory);
			}

			continue;
		}

		if (query->flags & FSQueryFlags_FileCount)
		{
			if (!query->extension)
			{
				results->FileCount++;
				continue;
			}

			size_t fileExtIter = 0;
			while (fileExtIter != fdNameLength)
			{
				if (fd.cFileName[fdNameLength - fileExtIter] == '.')
					break;

				fileExtIter++;
			}

			if (fileExtIter == fdNameLength)
				continue;

			if (!strcmp(&fd.cFileName[fdNameLength - fileExtIter], query->extension))
				results->FileCount++;
		}

		if (query->flags & FSQueryFlags_FileNames)
		{
			if (!query->extension)
			{
				UpdateQueryResults(query, &results->MatchingFiles, fd.cFileName, fdNameLength, FSTypeFlag_File);
				continue;
			}

			size_t fileExtIter = 0;

			while (fileExtIter != fdNameLength)
			{
				if (fd.cFileName[fdNameLength - fileExtIter] == '.')
					break;

				fileExtIter++;
			}

			if (fileExtIter == fdNameLength)
				continue;

			if (!strcmp(&fd.cFileName[fdNameLength - fileExtIter], query->extension))
			{
				UpdateQueryResults(query, &results->MatchingFiles, fd.cFileName, fdNameLength, FSTypeFlag_File);
			}
		}

		
	} while (FindNextFileA(hFind, &fd) != 0);

	Deallocate(newPath);
}

static const char** GetArrayFromQueryData(FSQueryData *data, const size_t count)
{
	const char** arr = Allocate(sizeof(char*) * count);

	size_t fileIter = 0;
	const FSQueryData* entry = data;
	do
	{
		if (!entry)
		{
			break;
		}

		arr[fileIter++] = entry->name;
		entry = entry->next;
	} while (entry != NULL);
	return arr;
}

static void DeallocateFSQueryDataNodes(FSQueryData* data)
{
	FSQueryData* entry = data;
	if (!entry)
		return;
	do
	{
		if (!entry)
		{
			break;
		}
		FSQueryData* current = entry;
		entry = entry->next;
		Deallocate(current);
	} while (entry != NULL);
}

i32 OSLIB_GetFileSize(const char *const filepath)
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

i32 OSLIB_GetDirectoryFileCount(const char *path)
{
	size_t pathLength = strlen(path);

	if (pathLength > (MAX_PATH - 3))
		return -1;

	FSQuery query;
	InitializeQuery(&query, FSQueryFlags_FileCount, path, pathLength, NULL, 0);

	FSQueryResults results;
	InitializeQueryResults(&results);
	QueryFileSystem(&query, &results);

	return results.FileCount;
}

i32 OSLIB_GetDirectorySubDirectoryCount(const char* path)
{
	size_t pathLength = strlen(path);

	if (pathLength > (MAX_PATH - 3))
		return -1;

	FSQuery query;
	InitializeQuery(&query, FSQueryFlags_DirectoryCount, path, pathLength, NULL, 0);

	FSQueryResults results;
	InitializeQueryResults(&results);
	QueryFileSystem(&query, &results);

	return results.DirectoryCount;
}

i32 OSLIB_GetDirectoryFileCountWithExtension(const char *path, const char *extension)
{
	size_t pathLength = strlen(path);
	size_t extensionLength = strlen(extension);

	if (pathLength > (MAX_PATH - extensionLength))
		return -1;

	FSQuery query;
	InitializeQuery(&query, FSQueryFlags_FileCount, path, pathLength, extension, extensionLength);

	FSQueryResults results;
	InitializeQueryResults(&results);
	QueryFileSystem(&query, &results);

	return results.FileCount;
}

const char ** OSLIB_GetFilesWithExtensionInDirectory(const char *path, const char *extension)
{
	size_t pathLength = strlen(path);
	size_t extensionLength = strlen(extension);

	if (pathLength > (MAX_PATH - extensionLength))
		return NULL;

	FSQuery query;
	InitializeQuery(&query, FSQueryFlags_FileNames | FSQueryFlags_FileCount, path, pathLength, extension, extensionLength);

	FSQueryResults results;
	InitializeQueryResults(&results);
	QueryFileSystem(&query, &results);

	if (results.FileCount == 0)
	{
		return NULL;
	}

	const char **files = GetArrayFromQueryData(results.MatchingFiles, results.FileCount);
	DeallocateFSQueryDataNodes(results.MatchingFiles);

	return files;
}

const char** OSLIB_GetSubDirectoriesForDirectory(const char* path)
{
	size_t pathLength = strlen(path);

	FSQuery query;
	InitializeQuery(&query, FSQueryFlags_DirectoryNames | FSQueryFlags_DirectoryCount, path, pathLength, NULL, 0);

	FSQueryResults results;
	InitializeQueryResults(&results);
	QueryFileSystem(&query, &results);

	const char** dirs = GetArrayFromQueryData(results.Directories, results.DirectoryCount);
	DeallocateFSQueryDataNodes(results.Directories);

	return dirs;
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

	u32 bytesWritten = 0;
	SetFilePointer(file, 0, NULL, FILE_END);
	WriteFile(file, buffer, bufferSize, &bytesWritten, NULL);
	error = GetLastError();

	CloseHandle(file);

	return bytesWritten;
}

i32 OSLIB_DeleteFile(const char* filepath)
{
	DeleteFileA(filepath);

	return 0;
}