#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

#include "config.h"
#include "filelist.h"
#include "github.h"

#define STDIN_BUFFER_SIZE 1024

char *user_input()
{
	char buffer[STDIN_BUFFER_SIZE], *content, *old;
	size_t contentSize = 1;

	content = (char *)malloc(sizeof(char) * STDIN_BUFFER_SIZE);
	if (content == NULL) {
		printf("Failed to read the user input\n");
		exit(EXIT_FAILURE);
	}
	content[0] = '\0';
	while (fgets(buffer, STDIN_BUFFER_SIZE, stdin)) {
		old = content;
		contentSize += strlen(buffer);
		content = realloc(content, contentSize);
		if (content == NULL) {
			printf("Failed to reallocate memory to get user input\n");
			exit(EXIT_FAILURE);
		}
		strcat(content, buffer);
	}
	return content;
}

char *basename(char *path)
{
	char *ptr, *base;
	int i, size;

	size = strlen(path);
	for (i = size - 1; i >= 0; i--) {
		if (path[i] == '/') {
			if (i == size - 1) {
				return NULL;
			}
			base = (char *)malloc((strlen(ptr) + 1) * sizeof(char));
			strcpy(base, ptr);
			return base;
		}
		ptr = &path[i];
	}

	base = (char *)malloc((size + 1) * sizeof(char));
	strcpy(base, path);
	return base;
}

char *default_name(const char *content)
{
	// This method is very silly, but sometimes is better than mark always as text
	char tmp[5];

	strncpy(tmp, content, 4);
	tmp[4] = '\0';
	if (strcmp(tmp, "diff") == 0) {
		return "default.diff";
	} else if (strstr(content, "<?php") != NULL) {
		return "default.php";
	} else if (strstr(content, "<?xml") != NULL) {
		return "default.xml";
	} else if (strstr(content, "#include <stdio.h>") != NULL) {
		return "default.c";
	} else if (strstr(content, "import java.") != NULL) {
		return "default.java";
	} else if (strstr(content, "<html>") != NULL) {
		return "default.html";
	}
	return "default.txt";
}

json_t *json_from_filelist(struct filelist *file)
{
	FILE *fp;
	int fsize;
	char *content;
	json_t *json;

	fp = fopen(file->filepath, "r");
	if (!fp) {
		return NULL;
	}
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	content = (char *)malloc((fsize + 1) * sizeof(char));
	if (content == NULL) {
		return NULL;
	}
	if (fread(content, 1, fsize, fp) == 0) {
		free(content);
		fclose(fp);
		return NULL;
	}
	fclose(fp);

	json = json_object();
	json_object_set_new(json, "content", json_string(content));
	return json;
}

void version()
{
	printf("%s\nVersion %s\n", PACKAGE_NAME, PACKAGE_VERSION);
}

void usage()
{
	printf("Gist It! Usage:\n");
	printf("  -v|--version              Show the application version\n");
	printf("  -h|--help                 Show this message\n");
	printf("  -d|--description <TEXT>   Send the text as gist description\n");
	printf("  -priv                     Mark the gist as private\n");
	printf("  -i <FILENAME>             Fake filename to send to GitHub using the application input\n");
	printf("  <FILE>                    The gist will include the <FILE> (you can specify multiple files)\n");
}

int main(int argc, char *argv[])
{
	json_t *j_post, *j_files, *j_file, *j_response, *j_url;
	json_error_t j_error;
	int i, is_public = 1;
	char *description = NULL, *fakename = NULL, *content;
	struct github_response *response;
	struct filelist *files = NULL, *current;

	// Check parameters
	for (i = 1; i < argc; i++) {
		// Version
		if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
			version();
			return EXIT_SUCCESS;
		}

		// Help/usage
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			usage();
			return EXIT_SUCCESS;
		}

		// Set the gist description
		if ((strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--description") == 0) && argv[i + 1] != NULL) {
			description = argv[++i];
			continue;
		}

		// Define the gist as private
		if (strcmp(argv[i], "-priv") == 0) {
			is_public = 0;
			continue;
		}

		// Get the gist filename
		if (strcmp(argv[i], "-i") == 0 && argv[i + 1] != NULL) {
			fakename = argv[++i];
			continue;
		}

		// Reading files
		if (argv[i][0] != '-') {
			if (files == NULL) {
				files = filelist_create(argv[i]);
			} else {
				filelist_add(files, argv[i]);
			}
			continue;
		}

		printf("Warning: Unknown parameter: %s\n", argv[i]);
	}

	// Creating the JSON list of files
	j_files = json_object();

	if (files == NULL) {
		content = user_input();
		if (fakename == NULL) {
			fakename = default_name(content);
		}

		j_file = json_object();
		json_object_set_new(j_file, "content", json_string(content));
		json_object_set_new(j_files, fakename, j_file);
	} else {
		current = files;
		do {
			j_file = json_from_filelist(current);
			if (j_file == NULL) {
				printf("Failed to open the file: %s\n", current->filepath);
				return EXIT_FAILURE;
			} else {
				json_object_set_new(j_files, current->filename, j_file);
			}
			current = current->next;
		} while(current != NULL);
	}

	j_post = json_object();
	if (description != NULL) {
		json_object_set_new(j_post, "description", json_string(description));
	}
	json_object_set_new(j_post, "public", is_public ? json_true() : json_false());
	json_object_set_new(j_post, "files", j_files);

	response = github_submit(j_post);
	if (response == NULL) {
		return EXIT_FAILURE;
	}
	j_response = json_loads(response->response_text, 0, &j_error);
	if (j_response == NULL) {
		printf("Failed to parse GitHub response\n");
		return EXIT_FAILURE;
	}
	j_url = json_object_get(j_response, "html_url");
	if (!json_is_string(j_url)) {
		printf("Failed to parse GitHub response\n");
		return EXIT_FAILURE;
	}
	printf("Gist URL: %s\n", json_string_value(j_url));

	return EXIT_SUCCESS;
}
