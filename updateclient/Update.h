#pragma once
#include <map>
#include <string>
#include "pthread.h"
#include "tinyxml2.h"
#ifdef WIN32
#include "curl.h"
#else
#include <curl/curl.h>
#endif

using namespace std;
using namespace tinyxml2;

class Update
{
private:
	struct UpdateFile 
	{
		enum OptType
		{
			OT_REMOVE,
			OT_KEEP,
			OT_UPDATE,
			OT_CREATE,
		};

		enum FileType
		{
			FT_DIR,
			FT_FILE,
		};

		UpdateFile()
			: opt_type(OT_REMOVE)
			, type(FT_DIR)
			, size(0)
		{
			md5[0] = '\0';
		}

		UpdateFile(FileType t, unsigned int s, const char* m)
			: opt_type(OT_REMOVE)
			, type(t)
			, size(s)
		{
			memcpy(md5, m, 32);
			md5[32] = '\0';
		}
		OptType opt_type;
		FileType type;
		unsigned int size;
		unsigned char md5[33];
	};

	typedef map<string, UpdateFile> UpdateMap;

	static void* UpdateThread(void* ptr);

public:
	Update(const char* local_update_path, const char* server_update_url)
		: m_update_size(0)
		, m_download_size(0)
		, m_download_thread(new pthread_t)
		, m_download_file(NULL)
	{
		m_curl = curl_easy_init();
		m_xml = new tinyxml2::XMLDocument;
		m_local_update_path = local_update_path;
		m_server_update_url = server_update_url;
	}

	~Update()
	{
		if (NULL != m_curl)
		{
			curl_easy_cleanup(m_curl);
		}
		if (m_xml)
		{
			delete m_xml;
		}
		if (m_download_thread)
		{
			delete m_download_thread;
		}
	}

	void LoadDir(UpdateMap& update_file_map, const string& parent_path, XMLElement* first_element);

	bool CheckUpdate();

	bool HasNewVersion()
	{
		return m_local_version != m_server_version;
	}

	void update();

public:
	CURL* m_curl;
	tinyxml2::XMLDocument* m_xml;
	string m_local_version;
	string m_server_version;

	UpdateMap m_local_update_map;
	UpdateMap m_server_update_map;
	string m_local_update_path;
	string m_server_update_url;
	string m_server_update_str;

	unsigned int m_update_size;
	unsigned int m_download_size;

	pthread_t* m_download_thread;
	FILE* m_download_file;

};

