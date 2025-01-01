#include <include/oslib/platform.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

i32 OSLIB_GetFileSize(const char *filepath)
{
    errno = 0;
    struct stat status;
    i32 res = stat(filepath, &status);
    
    if (res)
    {
        printf("stat error code: %d\n", errno);
        return -1;
    }

    return status.st_size;
}

i32 OSLIB_ReadBytesFromFile(const char *filepath, i8 *buffer, i32 bufferSize)
{
    errno = 0;
    FILE *file = fopen(filepath, "rb");
    if (file == NULL)
    {  
        printf("fopen error code: %d\n", errno);
        return -1;
    }

    i32 fileSize = OSLIB_GetFileSize(filepath);

    fread(buffer, 1, fileSize, file);
    fclose(file);

    return fileSize;
}

enum OSLIB_MatchCriteria
{
    MatchCriteria_None,
    MatchCriteria_Extension,
    MatchCriteria_IsDirectory,
    MatchCriteria_Count
};

static i32 OSLIB_GetDirectoryFileCountMatchingCriteria(const char *path, const enum OSLIB_MatchCriteria criteria, const char *pattern)
{
    DIR * d = opendir(path);
    struct dirent *it = NULL;

    if (d == NULL) return -1;

    size_t patternLength = strlen(pattern);

    i32 retval = 0;
    it = readdir(d);
    while(it != NULL)
    {
        switch (criteria)
        {
            case MatchCriteria_None:
            {
                ++retval;
                break;
            }
            case MatchCriteria_Extension:
            {
                size_t itNameLen = strlen(it->d_name);
		        size_t itExtIter = itNameLen;

                while (itExtIter != 0)
		        {
			        if (it->d_name[itExtIter] == '.')
				        break;

			        itExtIter--;
		        }

                if (!strcmp(&it->d_name[itExtIter], pattern))
                {
                    ++retval;
                }
                break;
            }
            case MatchCriteria_IsDirectory:
            {
                if (it->d_type == DT_DIR)
                {
                    ++retval;
                }
                break;
            }
        }
        it = readdir(d);
    }
    return retval;
}

i32 OSLIB_GetDirectoryFileCount(const char *path)
{
    return OSLIB_GetDirectoryFileCountMatchingCriteria(path, MatchCriteria_None, NULL);
}

i32 OSLIB_GetDirectorySubDirectoryCount(const char *path)
{
    return OSLIB_GetDirectoryFileCountMatchingCriteria(path, MatchCriteria_IsDirectory, NULL);
}

i32 OSLIB_GetDirectoryFileCountWithExtension(const char *path, const char *extension)
{
    return OSLIB_GetDirectoryFileCountMatchingCriteria(path, MatchCriteria_Extension, extension);
}

i32 OSLIB_WriteBytesToFile(const char *filepath, i8 *buffer, i32 bufferSize)
{
    FILE *file = fopen(filepath, "wb");

    if (file == NULL)
        return -1;
    
    i32 written = fwrite(buffer, 1, bufferSize, file);
    fclose(file);
    return written;
}

i32 OSLIB_AppendBytesToFile(const char *filepath, i8 *buffer, i32 bufferSize)
{
    FILE * file = fopen(filepath , "wb");
    fseek ( file , 0 , SEEK_END );
    i32 written = fwrite(buffer, 1, bufferSize, file);
    fclose ( file );
    return written;
}

const char ** OSLIB_GetFilesWithExtensionInDirectory(const char *filepath, const char *extension)
{
	size_t filepathLength = strlen(filepath);
	size_t extensionLength = strlen(extension);

    DIR * d = opendir(filepath);
    struct dirent *it = NULL;
    if (d == NULL)
        return NULL;

    i32 filesWithExtension = OSLIB_GetDirectoryFileCountWithExtension(filepath, extension);
    const char **filenames = Allocate(sizeof(char) * filesWithExtension);

    i32 fileCount = 0;
    it = readdir(d);
    while(it != NULL)
    {
		size_t filenameLength = strlen(it->d_name);
		size_t fileExtIter = filenameLength;

        while (fileExtIter != 0)
		{
			if (it->d_name[fileExtIter] == '.')
				break;

			fileExtIter--;
		}

        if (!strcmp(&it->d_name[fileExtIter], extension))
		{
			char *filepathString = Allocate(sizeof(char) * (filepathLength + filenameLength) + 1);
			memcpy(filepathString, filepath, sizeof(char) * filepathLength);
			memcpy(&filepathString[filepathLength], it->d_name, sizeof(char) * filenameLength);
			filepathString[filepathLength + filenameLength] = '\0';
			filenames[fileCount++] = filepathString;
		}

        it = readdir(d);
    }
    return filenames;
}

i32 OSLIB_DeleteFile(const char *filepath)
{
    return remove(filepath);
}