#include "github.h"
#include <jansson.h>
#include <curl/curl.h>

#define GITHUB_GIST_URL "https://api.github.com/gists?access_token=%s"
#define GITHUB_GIST_URL_ANONYMOUS "https://api.github.com/gists"

#define ENV_ACCESS_TOKEN_KEY "GISTIT_TOKEN"

struct github_response *github_submit(json_t *content)
{
	CURL *curl;
	CURLcode res;
	char url[100], *token;
	struct github_response *response;
	struct curl_slist *headers = NULL;
	long code;

	curl = curl_easy_init();
	if (curl) {
		response = (struct github_response *)malloc(1 * sizeof(struct github_response));
		response->response_text = (char *)malloc(1 * sizeof(char));
		response->response_text[0] = '\0';
		response->length = 0;

		headers = curl_slist_append(headers, "Content-Type: application/json");

		token = getenv(ENV_ACCESS_TOKEN_KEY);
		if (token != NULL) {
			sprintf(url, GITHUB_GIST_URL, token);
		} else {
			sprintf(url, GITHUB_GIST_URL_ANONYMOUS);
		}

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_dumps(content, 0));
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, github_response);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

		res = curl_easy_perform(curl);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
		if (code != 201) {
			printf("Failed to post to GitHub\n- code %ld\n- response %s\n", code, response->response_text);
			free(response->response_text);
			free(response);
			response = NULL;
		}
	}
	return response;
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
