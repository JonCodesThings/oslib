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

    if (d == NULL) return 0;

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
                    if (!(strlen(it->d_name) <= 2 && it->d_name[0] == '.'))
                    {
                        ++retval;
                    }
                }
                break;
            }
	    default: break;
        }
        it = readdir(d);
    }
    return retval;
}

static const char ** OSLIB_GetDirectoryStringsMatchingCriteria(const char *path, const enum OSLIB_MatchCriteria criteria, const char *pattern)
{
    DIR * d = opendir(path);
    struct dirent *it = NULL;

    if (d == NULL) return NULL;

    size_t pathLen = strlen(path);

    i32 filesMatching = OSLIB_GetDirectoryFileCountMatchingCriteria(path, criteria, pattern);
    u32 fileCount = 0;
    const char **filenames = Allocate(sizeof(char*) * filesMatching);
    it = readdir(d);
    while(it != NULL)
    {
        const char* nextFilename = NULL;
        switch (criteria)
        {
            case MatchCriteria_None:
            {
                nextFilename = it->d_name;
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
                    nextFilename = it->d_name;
                }
                break;
            }
            case MatchCriteria_IsDirectory:
            {
                if (it->d_type == DT_DIR)
                {
                    nextFilename = it->d_name;
                }
                break;
            }
	    case MatchCriteria_Count: break;
        }

        if (nextFilename)
        {
            i32 nextNameLength = strlen(nextFilename);
            if (!(nextNameLength <= 2 && nextFilename[0] == '.'))
            {
                i32 fullPathLength = pathLen + nextNameLength;
                i32 copiedIter = 0;
                fullPathLength += criteria == MatchCriteria_IsDirectory ? 2 : 1;
                char *filepathString = Allocate(sizeof(char) * fullPathLength);
			    memcpy(filepathString, path, sizeof(char) * pathLen);
                copiedIter += pathLen;
			    memcpy(&filepathString[copiedIter], it->d_name, sizeof(char) * nextNameLength);
                copiedIter += nextNameLength;
                if (criteria == MatchCriteria_IsDirectory)
                {
                    filepathString[copiedIter++] = '/';
                }
			    filepathString[copiedIter++] = '\0';
			    filenames[fileCount++] = filepathString;
            }
        }

        it = readdir(d);
    }
    return filenames;
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

const char ** OSLIB_GetFilesWithExtensionInDirectory(const char *path, const char *extension)
{
	return OSLIB_GetDirectoryStringsMatchingCriteria(path, MatchCriteria_Extension, extension);
}

const char ** OSLIB_GetSubDirectoriesForDirectory(const char *path)
{
    return OSLIB_GetDirectoryStringsMatchingCriteria(path, MatchCriteria_IsDirectory, NULL);
}

i32 OSLIB_DeleteFile(const char *filepath)
{
    return remove(filepath);
}
