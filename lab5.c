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
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

//static const char *hello_str = "Hello World!\n";
//static const char *hello_path = "/hello";
static const char *readme_str = "Student Artem Shcheglov 16150040";
static const char *example_str = "Hello world";
static char testtxt_str[40*2] = "";
static char mkdired_path[256] = " ";
static mode_t *mkdired_mode;

static int _getattr(const char *path, struct stat *stbuf)
{
    int res = 0;
    //stbuf->st_uid = getuid();
    stbuf->st_uid = 1000;
    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    }
    else if (strcmp(path, "/bar") == 0){
        stbuf->st_mode = S_IFDIR | 0705;
        stbuf->st_nlink = 2+1+1;
    }
    else if (strcmp(path, "/bar/bin") == 0){
        stbuf->st_mode = S_IFDIR | 0700;
        stbuf->st_nlink = 2;
    }
    else if (strcmp(path, "/bar/bin/echo") == 0){
        stbuf->st_mode = S_IFREG | 0555;
        stbuf->st_nlink = 1;

        struct stat buffer;
        stat("/bin/echo", &buffer);//Получение размера /bin/echo
        stbuf->st_size = buffer.st_size;
    }
    else if (strcmp(path, "/bar/bin/readme.txt") == 0){
        stbuf->st_mode = S_IFREG | 0400;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(readme_str);
    }
    else if (strcmp(path, "/bar/baz") == 0){
        stbuf->st_mode = S_IFDIR | 0644;
        stbuf->st_nlink = 2;
    }
    else if (strcmp(path, "/bar/baz/example") == 0){
        stbuf->st_mode = S_IFREG | 0222;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(example_str);
    }
    else if (strcmp(path, "/foo") == 0){
        stbuf->st_mode = S_IFDIR | 0233;
        stbuf->st_nlink = 2;
    }
    else if (strcmp(path, "/foo/test.txt") == 0){
        stbuf->st_mode = S_IFREG | 007;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(testtxt_str);
    }
    else if (strcmp(path, mkdired_path) == 0){
        stbuf->st_mode = mkdired_mode;
        stbuf->st_nlink = 2;
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
        filler(buf, "bar", NULL, 0);
        if (strcmp(mkdired_path, " ") != 0){
            filler(buf, mkdired_path+1, NULL, 0);
        }
        return 0;
    }
    else if (strcmp(path, "/bar") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, "bin", NULL, 0);
        filler(buf, "baz", NULL, 0);
        return 0;
    }
    else if (strcmp(path, "/bar/bin") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, "echo", NULL, 0);
        filler(buf, "readme.txt", NULL, 0);
        return 0;
    }
    else if (strcmp(path, "/bar/baz") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, "example", NULL, 0);
        return 0;
    }
    else if (strcmp(path, "/foo") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, "test.txt", NULL, 0);
        return 0;
    }
    else{
        return -ENOENT;
    }
}

static int _read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    size_t len;
    (void) fi;
    char *fileBuffer;
    if (strcmp(path, "/bar/bin/echo") == 0) {
        struct stat echo_stat;
        stat("/bin/echo", &echo_stat);//Получение размера /bin/echo
        len = echo_stat.st_size;

        FILE *f;
        unsigned char buffer[len];
        f = fopen("bin/echo", "r");
        fread(buffer, len, 1, f);
        fileBuffer = buffer;        
    }
    else if (strcmp(path, "/bar/bin/readme.txt") == 0) {
        len = strlen(readme_str);
        fileBuffer = readme_str;
    }
    else if (strcmp(path, "/bar/baz/example") == 0) {
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
        return 0;
    }
}

// fuse_operations hello_oper is redirecting function-calls to _our_ functions implemented above
static struct fuse_operations _oper = {
    .getattr        = _getattr,
    .readdir        = _readdir,
    .read           = _read,
};

int main(int argc, char *argv[])
{
    for (int i=0; i<61*2; i++){//<Любой текст на ваш выбор с количеством строк равным последним двум цифрам номера зачетки>
        strcat(testtxt_str, "1\n");
    }
    return fuse_main(argc, argv, &_oper, NULL);
}