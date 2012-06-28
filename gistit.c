#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "jansson.h"

#define VERSION "0.1"
#define STDIN_BUFFER_SIZE 1024
#define ENV_ACCESS_TOKEN_KEY "GISTIT_TOKEN"
#define GITHUB_GIST_URL "https://api.github.com/gists?access_token=%s"

struct github_response {
	char *response_text;
	size_t length;
};

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

size_t github_response(void *ptr, size_t size, size_t nmemb, void *stream)
{
	struct github_response *response = (struct github_response *)stream;
	size_t new_length = response->length + size * nmemb;

	response->response_text = realloc(response->response_text, new_length + 1);
	memcpy(response->response_text + response->length, ptr, size * nmemb);
	response->response_text[new_length] = '\0';
	response->length = new_length;

	return size * nmemb;
}

struct github_response *github_submit(json_t *content)
{
	CURL *curl;
	CURLcode res;
	char url[100], *token;
	struct github_response *response;
	long code;

	curl = curl_easy_init();
	if (curl) {
		token = getenv(ENV_ACCESS_TOKEN_KEY);
		if (token == NULL) {
			printf("Gist It! Token is not defined. Please define the %s environement\n", ENV_ACCESS_TOKEN_KEY);
			return NULL;
		}

		response = (struct github_response *)malloc(1 * sizeof(struct github_response));
		response->response_text = (char *)malloc(1 * sizeof(char));
		response->response_text[0] = '\0';
		response->length = 0;

		sprintf(url, GITHUB_GIST_URL, token);
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_dumps(content, 0));
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, github_response);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

		res = curl_easy_perform(curl);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
		curl_easy_cleanup(curl);
		if (code != 201) {
			printf("Failed to post to GitHub\n- code %ld\n- response %s\n", code, response->response_text);
			//free(&response->response_text);
			//free(response);
			response = NULL;
		}
	}
	return response;
}

void version()
{
	printf("Gist It!\nVersion %s\n", VERSION);
}

void usage()
{
	printf("Gist It! Usage:\n");
	printf("  -v|--version              Show the application version\n");
	printf("  -h|--help                 Show this message\n");
	printf("  -d|--description <TEXT>   Send the text as gist description\n");
	printf("  -priv                     Mark the gist as private\n");
	printf("  -f <FILE>                 The gist will be <FILE>\n");
	printf("  -i <FILENAME>             Fake filename to send to GitHub using the application input\n");
}

int main(int argc, char *argv[])
{
	json_t *j_post, *j_files, *j_file, *j_response, *j_url;
	json_error_t j_error;
	int i, is_public = 1, fsize;
	char *description = NULL, *filename = NULL, *fakename = NULL, *content;
	FILE *fp;
	struct github_response *response;

	// Check parameters
	for (i = 1; i < argc; i++) {
		// Version
		if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
			version();
			return 0;
		}

		// Help/usage
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			usage();
			return 0;
		}

		// Set the gist description
		if ((strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--description") == 0) && argv[i + 1] != NULL) {
			description = argv[i + 1];
			i += 1;
			continue;
		}

		// Define the gist as private
		if (strcmp(argv[i], "-priv") == 0) {
			is_public = 0;
			continue;
		}

		// Leave -f to read from filename
		if (strcmp(argv[i], "-f") == 0 && argv[i + 1] != NULL) {
			filename = argv[i + 1];
			continue;
		}

		if (strcmp(argv[i], "-i") == 0 && argv[i + 1] != NULL) {
			fakename = argv[i + 1];
			continue;
		}
	}

	if (filename == NULL) {
		if (fakename == NULL) {
			fakename = (char *)malloc(15 * sizeof(char));
			strcpy(fakename, "default.txt");
		}
		content = user_input();
	} else {
		fp = fopen(filename, "r");
		if (!fp) {
			printf("Failed to open the specified file\n");
			return EXIT_FAILURE;
		}
		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		content = (char *)malloc((fsize + 1) * sizeof(char));
		if (content == NULL) {
			printf("Error allocating space for read the file. Seems the file is too big\n");
			return EXIT_FAILURE;
		}
		fread(content, 1, fsize, fp);
		fclose(fp);
		// @todo change filename variable to catch only the basename. Ie, -f ../file.txt must be converted to file.txt only
		fakename = filename;
	}

	j_file = json_object();
	json_object_set_new(j_file, "content", json_string(content));

	j_files = json_object();
	json_object_set_new(j_files, fakename, j_file);

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
