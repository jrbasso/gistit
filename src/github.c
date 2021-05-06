#include "github.h"
#include "config.h"
#include <jansson.h>
#include <curl/curl.h>

#define GITHUB_GIST_URL "https://api.github.com/gists"
#define ENTERPRISE_GITHUB_GIST_URL "%s/gists"

#define ENV_ACCESS_TOKEN_KEY "GISTIT_TOKEN"
#define ENV_GITHUB_API_URL "GISTIT_API_URL"

struct github_response *github_submit(json_t *content)
{
	CURL *curl;
	CURLcode res;
	char url[100], userAgent[40], authorizationHeader[100], *token, *api_url;
	struct github_response *response = NULL;
	struct curl_slist *headers = NULL;
	long code;

	curl = curl_easy_init();
	if (curl) {
		response = (struct github_response *)malloc(1 * sizeof(struct github_response));
		response->response_text = (char *)malloc(1 * sizeof(char));
		response->response_text[0] = '\0';
		response->length = 0;

		headers = curl_slist_append(headers, "Content-Type: application/json");
		sprintf(userAgent, "User-Agent: %s/%s", PACKAGE_NAME, PACKAGE_VERSION);
		headers = curl_slist_append(headers, userAgent);
		api_url = getenv(ENV_GITHUB_API_URL);
		if (api_url != NULL) {
			snprintf(url, sizeof(url), ENTERPRISE_GITHUB_GIST_URL, api_url);
		} else {
			sprintf(url, GITHUB_GIST_URL);
		}

		token = getenv(ENV_ACCESS_TOKEN_KEY);
		if (token != NULL) {
			snprintf(authorizationHeader, sizeof(authorizationHeader), "Authorization: token %s", token);
			headers = curl_slist_append(headers, authorizationHeader);
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
