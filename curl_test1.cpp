#include <cstdlib>
#include <iostream>
#include <curl/curl.h>

using namespace std;

int main(int argc, char **argv)
{
	//初始化
	curl_global_init(CURL_GLOBAL_ALL);
	CURLM *multi_handle = NULL;
	CURL *easy_handle1 = NULL;
	CURL *easy_handle2 = NULL;

	extern size_t save_sina_page(void *buffer, size_t size, size_t count, void *user_p);
	extern size_t save_sohu_page(void *buffer, size_t size, size_t count, void *user_p);
	FILE *fp_sina = fopen("sina.html", "ab+");
	FILE *fp_sohu = fopen("sohu.html", "ab+");

	multi_handle = curl_multi_init();

	//设置easy handle
	easy_handle1 = curl_easy_init();
	curl_easy_setopt(easy_handle1, CURLOPT_URL, "http://www.sina.com.cn");
	curl_easy_setopt(easy_handle1, CURLOPT_WRITEFUNCTION, &save_sina_page);
	curl_easy_setopt(easy_handle1, CURLOPT_WRITEDATA, fp_sina);

	easy_handle2 = curl_easy_init();
	curl_easy_setopt(easy_handle2, CURLOPT_URL, "http://www.sohu.com");
	curl_easy_setopt(easy_handle2, CURLOPT_WRITEFUNCTION, &save_sohu_page);

	//添加到multi stack
	curl_multi_add_handle(multi_handle, easy_handle1);
	curl_multi_add_handle(multi_handle, easy_handle2);


	int running_handle_count;
	while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(multi_handle, &running_handle_count))
	{
		cout << running_handle_count << endl;
	}

	while (running_handle_count)
	{
		timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		int max_fd;
		fd_set fd_read;
		fd_set fd_write;
		fd_set fd_except;

		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		FD_ZERO(&fd_except);

		curl_multi_fdset(multi_handle, &fd_read, &fd_write, &fd_except, &max_fd);
		int return_code = select(max_fd + 1, &fd_read, &fd_write, &fd_except, &tv);
		if (-1 == return_code)
		{
			cerr << "select error." << endl;
			break;
		}
		else
		{
			while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(multi_handle, &running_handle_count))
			{
				cout << running_handle_count << endl;
			}
		}
	}

	//释放资源
	fclose(fp_sina);
	fclose(fp_sohu);
	curl_easy_cleanup(easy_handle1);
	curl_easy_cleanup(easy_handle2);
	curl_multi_cleanup(multi_handle);
	curl_global_cleanup();

	return 0;
}

size_t save_sina_page(void *buffer, size_t size, size_t count, void *user_p)
{
	return fwrite(buffer, size, count, (FILE *)user_p);
}

size_t save_sohu_page(void *buffer, size_t size, size_t count, void *user_p)
{
	return fwrite(buffer, size, count, (FILE *)user_p);
}
