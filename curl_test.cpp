#include <stdio.h>
#include <curl/curl.h>
#include <iostream>

using namespace std;

size_t process_data(void *buffer, size_t size, size_t nmemb, void *user_p)
{
	static int count = 0;
	FILE *fp = (FILE *)user_p;
	size_t return_size = fwrite(buffer, size, nmemb, fp);
	return return_size;
}

int main()
{
	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != code)
	{
		return -1;
	}

	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		curl_global_cleanup();
		return -1;
	}
	FILE *fp = fopen("loading-2.png", "w");
	curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.1.7/update/loading-2.png");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &process_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_perform(curl);


	fclose(fp);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
	printf("hello\n");
	return 0;
}

