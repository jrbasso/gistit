#include <stdlib.h>
#include "filelist.h"

struct filelist *filelist_create(char *filepath)
{
	struct filelist *list = (struct filelist *)malloc(sizeof(struct filelist));

	list->filepath = filepath;
	list->filename = basename(filepath);
	list->next = NULL;
	return list;
}

void filelist_add(struct filelist *list, char *filepath)
{
	struct filelist *current;

	current = list;
	while (current->next != NULL) {
		current = current->next;
	}
	current->next = filelist_create(filepath);
}
