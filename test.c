/*
	gcc -Wall test.c `pkg-config fuse3 --cflags --libs` -o l5
*/

#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

static const char *bar_path = "/bin/bar";
static const char *baz_path = "/bin/baz";
static const char *bin_path = "/bin";
static const char *foo_path = "/foo";
static const char *cat_path = "/bin/baz/cat";
static const char *system_cat_path = "/bin/cat";
char testText[106];
static int rm = 0, rm1 = 0;

static struct options
{
	const char *filename;
	char *contents;
} readme, example, test;

size_t filesize(const char *filename) {
	struct stat st;
	size_t retval = 0;
	if (stat(filename, &st))
		printf("cannot stat %s\n", filename);
	else 
		retval = st.st_size;
	return retval;
}

static int file_getattr(const char *path, struct stat *stbuf,
		struct fuse_file_info *fi)
{
	(void) fi;

	memset(stbuf, 0, sizeof(struct stat));
	if (!strcmp(path, "/")) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (!strcmp(path, bin_path)) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (!strcmp(path, bar_path) && !rm1) {
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
	} else if (!strcmp(path, baz_path)) {
		stbuf->st_mode = S_IFDIR | 0744;
		stbuf->st_nlink = 2;
	} else if (!strcmp(path, cat_path)) {
		stbuf->st_mode = S_IFREG | 0677;
		stbuf->st_nlink = 1;
		stbuf->st_size = filesize(system_cat_path);
	} else if (!strcmp(path, example.filename)) {
		stbuf->st_mode = S_IFREG | 0200;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(example.contents);
	} else if (!strcmp(path, readme.filename)) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(readme.contents);
	} else if (!strcmp(path, foo_path)&& !rm) {
		stbuf->st_mode = S_IFDIR | 0665;
		stbuf->st_nlink = 2;
	} else if (!strcmp(path, test.filename)) {
		stbuf->st_mode = S_IFREG | 0556;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(test.contents);
	} else return -ENOENT;

	return 0;
}

static int file_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi,
		enum fuse_readdir_flags flags)
{
	(void)offset;
	(void)fi;
	(void)flags;

	if (!strcmp(path, "/")) {	
		filler(buf, ".", NULL, 0, 0);
		filler(buf, "..", NULL, 0, 0);
		filler(buf, bin_path + 1, NULL, 0, 0);
		filler(buf, foo_path + 1, NULL, 0, 0);
	} else if (!strcmp(path, bin_path)) {
		filler(buf, ".", NULL, 0, 0);
		filler(buf, "..", NULL, 0, 0);
	    filler(buf, "/bar" + 1, NULL, 0, 0);
		filler(buf, "/baz" + 1, NULL, 0, 0);
	} else if (!strcmp(path, baz_path)) {
		filler(buf, ".", NULL, 0, 0);
		filler(buf, "..", NULL, 0, 0);
		filler(buf, "/cat" + 1, NULL, 0, 0);
		filler(buf, "/example" + 1, NULL, 0, 0);
		filler(buf, "/readme.txt" + 1, NULL, 0, 0);	
	} else if (!strcmp(path, foo_path) && !rm) {
		filler(buf, ".", NULL, 0, 0);
		filler(buf, "..", NULL, 0, 0);
		filler(buf, "/test.txt" + 1, NULL, 0, 0);
	}
	else if(!strcmp(path, bar_path) && !rm1){
		filler(buf, ".", NULL, 0, 0);
		filler(buf, "..", NULL, 0, 0);
	}
	 else
		return -ENOENT;

	return 0;
}

static int file_read(const char *path, char *buf, size_t size, off_t offset,
			struct fuse_file_info *fi)
{
	size_t len;
	(void)fi;
	struct options to_read;

	if (strcmp(path, cat_path) == 0)
	{
		FILE *fcp = fopen(system_cat_path, "rb");
		unsigned char c;
		while (fread(&c, 1, 1, fcp))
			putchar(c);
		fclose(fcp);
		return size;
	}
	else if (strcmp(path, example.filename) == 0) {
		to_read = example;
	}
	else if (strcmp(path, readme.filename) == 0) {
		to_read = readme;
	}
	else if (strcmp(path, test.filename) == 0) {
		to_read = test;
	}
	else 
		return -ENOENT;

	len = strlen(to_read.contents);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, to_read.contents + offset, size);
	}
	else
		size = 0;
	
	return size;
}
static int file_rmdir(const char *path) {
	if (!strcmp(path, "/"))	
		return -ENOTEMPTY;
	else if (!strcmp(path, bin_path))
		return -ENOTEMPTY;
	else if (!strcmp(path, bar_path) && !rm1) {
		rm1 = 1;
        return 0;
	} else if (!strcmp(path, baz_path))
		return -ENOTEMPTY;	
	else if (!strcmp(path, foo_path) && !rm){
		rm = 1;
		return 0;
	}
	else
		return -ENOENT;	
}


static struct fuse_operations operations = 
{
	.read = file_read,
    //.rmdir	= file_rmdir,
	.readdir = file_readdir,
	.getattr = file_getattr	

};

int main(int argc, char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	example.filename = strdup("/bin/baz/example");
	example.contents = strdup("Hello world\n");
	readme.filename = strdup("/bin/baz/readme.txt");
	readme.contents = strdup("Student Artem, 16120053\n");
	test.filename = strdup("/foo/test.txt");
    for(int i=0;i<53;i++){
     testText[i*2] = 'a';
     testText[i*2+1] = '\n';
}
      testText[105] = '\0';
       test.contents = strdup(testText);
    

	return fuse_main(args.argc, args.argv, &operations, NULL);
}