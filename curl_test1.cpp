#include <stdlib.h>
#include <curl/curl.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

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
	const char* url_1 = "http://192.168.1.8/update/vld.ini";
	curl_easy_setopt(curl_1, CURLOPT_URL, url_1);
	curl_easy_setopt(curl_1, CURLOPT_PRIVATE, url_1);
	curl_easy_setopt(curl_1, CURLOPT_WRITEFUNCTION, &write1);
	curl_easy_setopt(curl_1, CURLOPT_WRITEDATA, fp1);

	FILE *fp2 = fopen("loading-2.png", "w");
	assert(fp2 != NULL);
	CURL* curl_2 = curl_easy_init();
	assert(curl_2 != NULL);
	const char* url_2 = "http://192.168.1.7/update/loading-2a.png";
	curl_easy_setopt(curl_2, CURLOPT_URL, url_2);
	curl_easy_setopt(curl_2, CURLOPT_PRIVATE, url_2);
	curl_easy_setopt(curl_2, CURLOPT_WRITEFUNCTION, &write2);
	curl_easy_setopt(curl_2, CURLOPT_WRITEDATA, fp2);

	CURLM* multi_handle = curl_multi_init();
	assert(multi_handle != NULL);

	curl_multi_add_handle(multi_handle, curl_1);
	curl_multi_add_handle(multi_handle, curl_2);


	int running_count = -1;
	fd_set rset;
	fd_set wset;
	fd_set eset;
	long timeout;
	timeval tv;
	int max_fd = -1;
	CURLMsg* msg;
	int count;
	while (running_count != 0)
	{
		printf("start perform\n");
		curl_multi_perform(multi_handle, &running_count);
		if (running_count != 0)
		{
			FD_ZERO(&rset);
			FD_ZERO(&wset);
			FD_ZERO(&eset);
			if (curl_multi_fdset(multi_handle, &rset, &wset, &eset, &max_fd) != 0)
			{
				fprintf(stderr, "E: curl_multi_fdset\n");
				return EXIT_FAILURE;
			}
			if (curl_multi_timeout(multi_handle, &timeout))
			{
				fprintf(stderr, "E: curl_multi_timeout\n");
				return EXIT_FAILURE;
			}
			if (timeout == -1)
			{
				timeout = 100;
			}
			if (max_fd == -1)
			{
				printf("sleep\n");
#ifdef WIN32
				Sleep(timeout);
#else
				sleep(timeout / 1000);
#endif
			}
			else
			{
				tv.tv_sec = timeout / 1000;
				tv.tv_usec = (timeout % 1000) * 1000;
				if (select(max_fd + 1, &rset, &wset, &eset, &tv) < 0)
				{
					fprintf(stderr, "E : select(%i,,,,%li): %i: %s\n", max_fd + 1, timeout, errno, strerror(errno));
					return EXIT_FAILURE;
				}
			}
		}
	}
	while ((msg = curl_multi_info_read(multi_handle, &count)) != NULL)
	{
		fprintf(stdout, "count %d\n", count);
		if (msg->msg == CURLMSG_DONE)
		{
			char* url;
			CURL* e = msg->easy_handle;
			if (curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &url) != 0)
			{
				fprintf(stderr, "curl_easy_getinfo\n");
			}
			long ret_code = 0;
			assert(curl_easy_getinfo(e, CURLINFO_RESPONSE_CODE, &ret_code) == 0);
			fprintf(stdout, "ret code %d\n", ret_code);
			fprintf(stderr, "R: %d - %s <%s>\n", msg->data.result, curl_easy_strerror(msg->data.result), url);
			curl_multi_remove_handle(multi_handle, e);
			curl_easy_cleanup(e);
		}
		else
		{
			fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
		}
	}

	//释放资源
	fclose(fp1);
	fclose(fp2);
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
