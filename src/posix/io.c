#include <include/oslib/platform.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

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

i32 OSLIB_GetDirectoryFileCount(const char *filepath)
{
    DIR * d = opendir(filepath);
    struct dirent *it = NULL;

    if (d == NULL) return -1;

    i32 retval = 0;
    it = readdir(d);
    while(it != NULL)
    {
        ++retval;
        it = readdir(d);
    }
    return retval;
}

i32 OSLIB_GetDirectoryFileCountWithExtension(const char *filepath, const char *extension)
{

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

char ** OSLIB_GetFilesWithExtensionInDirectory(const char *filepath, const char *extension)
{
    
}