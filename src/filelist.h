
struct filelist {
	char *filepath;
	char *filename;
	struct filelist *next;
};

extern char *basename(char *path);

struct filelist *filelist_create(char *filepath);
void filelist_add(struct filelist *list, char *filepath);
