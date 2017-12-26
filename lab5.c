/*
    Вариант 5:
    /
    |--bin/700
    |   `--date/700
    `--foo/441
        |--bar/664
        |--test.txt/000
        |--example/200
        `--baz/244
            `--readme.txt/411
*/

#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


static const char *readme_str = "Student Artem Shcheglov 16150040";
static const char *example_str = "Hello world";
static char testtxt_str[40*2] = "";

static int _getattr(const char *path, struct stat *stbuf)
{
    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    }
    else if (strcmp(path, "/bin") == 0){
        stbuf->st_mode = S_IFDIR | 0700;
        stbuf->st_nlink = 2+1;
    }
    else if (strcmp(path, "/bin/date") == 0){
        stbuf->st_mode = S_IFREG | 0700;
        stbuf->st_nlink = 1;

	struct stat buffer;
        stat("/bin/date", &buffer); //Получение размера /bin/date
        stbuf->st_size = buffer.st_size;
    }
    else if (strcmp(path, "/foo") == 0){
        stbuf->st_mode = S_IFDIR | 0441;
        stbuf->st_nlink = 2+1+1+1+1;

        
    }
    else if (strcmp(path, "/foo/bar") == 0){
        stbuf->st_mode = S_IFDIR | 0664;
        stbuf->st_nlink = 2;
    }
    else if (strcmp(path, "/foo/test.txt") == 0){
        stbuf->st_mode = S_IFREG | 0000;
        stbuf->st_nlink = 1;
	stbuf->st_size = strlen(testtxt_str);
    }
    else if (strcmp(path, "/foo/example") == 0){
        stbuf->st_mode = S_IFREG | 0200;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(example_str);
    }
    else if (strcmp(path, "/foo/baz") == 0){
        stbuf->st_mode = S_IFDIR | 0244;
        stbuf->st_nlink = 2+1;
    }
    else if (strcmp(path, "/foo/baz/readme.txt") == 0){
        stbuf->st_mode = S_IFREG | 0777; //411
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(readme_str);
    }
    else{
        res = -ENOENT;
    }
            
    return res;
}
static int _readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
    (void) offset;
    (void) fi;
    if (strcmp(path, "/") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, "foo", NULL, 0);
        filler(buf, "bin", NULL, 0);

        return 0;
    }
    else if (strcmp(path, "/foo") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, "bar", NULL, 0);
	filler(buf, "test.txt", NULL, 0);
	filler(buf, "example", NULL, 0);
        filler(buf, "baz", NULL, 0);
        return 0;
    }
    else if (strcmp(path, "/bin") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, "date", NULL, 0);
        return 0;
    }
    else if (strcmp(path, "/foo/bar") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        return 0;
    }
    else if (strcmp(path, "/foo/baz") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, "readme.txt", NULL, 0);
        return 0;
    }
    else{
        return -ENOENT;
    }
}

static int _open(const char *path, struct fuse_file_info *fi){
	return 0;
}

static int _write (const char *path, const char *buf, size_t size, off_t offset,struct fuse_file_info *fi)
    {
        (void) buf;
        (void) offset;
        (void) fi;
        size_t len;
        char *fileBuffer;

        if (strcmp(path, "/foo/baz/readme.txt") == 0) {
           len = strlen(readme_str);
           fileBuffer = readme_str;
        }
        else if (strcmp(path, "/foo/example") == 0) {
            len = strlen(example_str);
            fileBuffer = example_str;
        }
        else if (strcmp(path, "/foo/test.txt") == 0) {
            len = strlen(testtxt_str);
            fileBuffer = testtxt_str;
            }

        if(offset+ size <= len)
		{ 
			for(int i=0;i<size;i++)
			{
				fileBuffer[offset+i] = buf[i];
			}
			return size;
		}else	
		{
			char *temp = (char*)malloc(offset + size);
			for(int i=0;i<len;i++)
			{
				temp[i] = fileBuffer[i];
			}

			for(int i=0;i<size;i++)
			{
				temp[offset+i] = buf[i];
			}
			fileBuffer = temp;
			len = offset + size;
			return size;
		}
        return -1;
            
    }

static int _read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    size_t len;
    (void) fi;
    char *fileBuffer;
    if (strcmp(path, "/bin/date") == 0) {
        struct stat date_stat;
        stat("/bin/date", &date_stat); //Получение размера /bin/date
        len = date_stat.st_size;

        FILE *f;
        unsigned char buffer[len];
        f = fopen("bin/date", "r");
        fread(buffer, len, 1, f);
        fileBuffer = buffer;        
    }
    else if (strcmp(path, "/foo/baz/readme.txt") == 0) {
        len = strlen(readme_str);
        fileBuffer = readme_str;
    }
    else if (strcmp(path, "/foo/example") == 0) {
        len = strlen(example_str);
        fileBuffer = example_str;
    }
    else if (strcmp(path, "/foo/test.txt") == 0) {
        len = strlen(testtxt_str);
        fileBuffer = testtxt_str;
    }
    else{
        return -ENOENT;
    }

    if (offset < len){
        if (offset + size > len){
            size = len-offset;
        }
        memcpy(buf, fileBuffer+offset, size);
        return size;
    }
    else{
        return -1;
    }
}
int my_setxattr (const char *b, const char *c, const char *d, size_t e, int f)
{
return 0;
}

int my_chown (const char *a, uid_t b, gid_t c)
{
return 0;
}
int my_chmod (const char *a, mode_t b)
{
return 0;
}
int my_truncate (const char *a, off_t b)
{
return 0;
}
int my_utime (const char *a, struct utimbuf *b)
{
return 0;
}

static struct fuse_operations _oper = {
    .getattr        = _getattr,
    .readdir        = _readdir,
    .read           = _read,
    .setxattr   = my_setxattr,
    .chmod = my_chown,
    .chown = my_chmod,
    .truncate = my_truncate,
    .utime =my_utime,
    .open           = _open,
    .write          = _write
};

int main(int argc, char *argv[])
{
    for (int i=0; i<40*2; i++){//<Любой текст на ваш выбор с количеством строк равным последним двум цифрам номера зачетки>
        strcat(testtxt_str, "1\n");
    }
    return fuse_main(argc, argv, &_oper, NULL);
}
