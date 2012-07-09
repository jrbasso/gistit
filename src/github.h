#include <string.h>
#include <jansson.h>

struct github_response {
	char *response_text;
	size_t length;
};

struct github_response *github_submit(json_t *content);
size_t github_response(void *ptr, size_t size, size_t nmemb, void *stream);