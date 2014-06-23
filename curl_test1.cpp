#include <cstdlib>
#include <iostream>
#include <curl/curl.h>
#include <assert.h>

using namespace std;

size_t write1(void *buffer, size_t size, size_t count, void *user_p);
size_t write2(void *buffer, size_t size, size_t count, void *user_p);
int main(int argc, char **argv)
{
	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);
	if (code != CURLE_OK)
	{
		printf("global init\n");
		return -1;
	}
	FILE *fp1 = fopen("vld.ini", "w");
	assert(fp1 != NULL);
	CURL* curl_1 = curl_easy_init();
	assert(curl_1 != NULL);
	curl_easy_setopt(curl_1, CURLOPT_URL, "http://192.168.1.7/update/vld.ini");
	curl_easy_setopt(curl_1, CURLOPT_WRITEFUNCTION, &write1);
	curl_easy_setopt(curl_1, CURLOPT_WRITEDATA, fp1);

	FILE *fp2 = fopen("loading-2.png", "w");
	assert(fp2 != NULL);
	CURL* curl_2 = curl_easy_init();
	assert(curl_2 != NULL);
	curl_easy_setopt(curl_2, CURLOPT_URL, "http://192.168.1.7/update/loading-2.png");
	curl_easy_setopt(curl_2, CURLOPT_WRITEFUNCTION, &write2);
	curl_easy_setopt(curl_2, CURLOPT_WRITEDATA, fp2);

	CURLM* multi_handle = curl_multi_init();
	assert(multi_handle != NULL);

	curl_multi_add_handle(multi_handle, curl_1);
	curl_multi_add_handle(multi_handle, curl_2);


	int running_handle_count = -1;
	while (curl_multi_perform(multi_handle, &running_handle_count) == CURLM_CALL_MULTI_PERFORM)
	{
		printf("perform...\n");
	}
	do
	{
		timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		int max_fd = -1;
		fd_set fd_read;
		fd_set fd_write;
		fd_set fd_except;

		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		FD_ZERO(&fd_except);

		curl_multi_fdset(multi_handle, &fd_read, &fd_write, &fd_except, &max_fd);
		if (max_fd == -1)
		{
			printf("max fd\n");
			break;
		}
		int return_code = select(max_fd + 1, &fd_read, &fd_write, &fd_except, &tv);
		switch (return_code)
		{
			case -1:
				cerr<<"select error"<<endl;
				break;
			case 0:
				break;
			default:
				printf("perfmorm\n");
				while (curl_multi_perform(multi_handle, &running_handle_count) == CURLM_CALL_MULTI_PERFORM)
				{
					printf("perform...\n");
				}
				printf("running %d\n", running_handle_count);
				break;
		}
	} while (running_handle_count > 0);

	//释放资源
	fclose(fp1);
	fclose(fp2);
	curl_easy_cleanup(curl_1);
	curl_easy_cleanup(curl_2);
	curl_multi_cleanup(multi_handle);
	curl_global_cleanup();

	return 0;
}

size_t write1(void *buffer, size_t size, size_t count, void *user_p)
{
	return fwrite(buffer, size, count, (FILE *)user_p);
}

size_t write2(void *buffer, size_t size, size_t count, void *user_p)
{
	return fwrite(buffer, size, count, (FILE *)user_p);
}
