#include "Update.h"
#include <stdlib.h>
#ifdef WIN32
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

static size_t WriteVersion(void* buffer, size_t size, size_t nmemb, void* userdata)
{
	string *data = (string*)userdata;
	data->append((char*)buffer, size * nmemb);

	return (size * nmemb);
}

static size_t WriteUpdate(void* buffer, size_t size, size_t nmemb, void* userdata)
{
	string *data = (string*)userdata;
	data->append((char*)buffer, size * nmemb);

	return (size * nmemb);
}

void Update::LoadDir(UpdateMap& update_map, const string& parent_path, tinyxml2::XMLElement* first_element)
{
	while (NULL != first_element)
	{
		if (XMLUtil::StringEqual(first_element->Value(), "DIR"))
		{
			tinyxml2::XMLElement* name_e = first_element->FirstChildElement();
			const string path = parent_path.empty() ? name_e->GetText() : parent_path + '/' + name_e->GetText();
			update_map.insert(make_pair(path, UpdateFile()));
			LoadDir(update_map, path, name_e->NextSiblingElement());
		}
		else
		{
			tinyxml2::XMLElement* name_e = first_element->FirstChildElement();
			tinyxml2::XMLElement* size_e = name_e->NextSiblingElement();
			tinyxml2::XMLElement* md5_e = size_e->NextSiblingElement();
			const string path = parent_path.empty() ? name_e->GetText() : parent_path + '/' + name_e->GetText();
			update_map.insert(make_pair(path, UpdateFile(UpdateFile::FT_FILE, atoi(size_e->GetText()), md5_e->GetText())));
		}
		first_element = first_element->NextSiblingElement();
	}
}

bool Update::CheckUpdate()
{
	FILE* version_file = fopen((m_local_update_path + "/version.txt").c_str(), "rb");
	if (version_file)
	{
		char version[128] = {0};
		fread(version, 1, 128, version_file);
		m_local_version = version;
		fclose(version_file);
	}
	curl_easy_setopt(m_curl, CURLOPT_URL, (m_server_update_url + "/version.txt").c_str());
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &WriteVersion);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_server_version);
	if (0 != curl_easy_perform(m_curl))
	{
		return false;
	}
	if (m_local_version == m_server_version)
	{
		return true;
	}

	if (0 == m_xml->LoadFile((m_local_update_path + "/update.xml").c_str()))
	{
		m_local_update_map.clear();
		LoadDir(m_local_update_map, "", m_xml->RootElement()->FirstChildElement());
	}

	curl_easy_setopt(m_curl, CURLOPT_URL, (m_server_update_url + "/update.xml").c_str());
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &WriteUpdate);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_server_update_str);
	if (0 != curl_easy_perform(m_curl))
	{
		return false;
	}
	if (0 != m_xml->Parse(m_server_update_str.c_str()))
	{
		return false;
	}
	m_server_update_map.clear();
	LoadDir(m_server_update_map, "", m_xml->RootElement()->FirstChildElement());

	return true;
}
static size_t WriteDownload(void* buffer, size_t size, size_t nmemb, void* userdata)
{
	Update* update = (Update*)userdata;
	update->m_download_size += size * nmemb;
	return fwrite(buffer, size, nmemb, update->m_download_file);
}

void* Update::UpdateThread(void* ptr)
{
	Update* update = (Update*)ptr;
	CURL* curl  = curl_easy_init();
	for (UpdateMap::const_iterator it = update->m_local_update_map.begin(); it != update->m_local_update_map.end(); ++it)
	{
		if (UpdateFile::OT_REMOVE == it->second.opt_type)
		{
			remove((update->m_local_update_path + '/' + it->first).c_str());
		}
		else if (UpdateFile::OT_UPDATE == it->second.opt_type || UpdateFile::OT_CREATE == it->second.opt_type)
		{
			const string full_path = update->m_local_update_path + '/' + it->first;
			int start_pos = update->m_local_update_path.length();
			while ((start_pos = full_path.find('/', start_pos + 1)) != string::npos)
			{
				string sub_path = full_path.substr(0, start_pos);
				if (-1 == access(sub_path.c_str(), 0))
				{
#ifdef WIN32
					mkdir(sub_path.c_str());
#else
					mkdir(sub_path.c_str(), S_IRWXU);
#endif
				}
			}
			if (UpdateFile::FT_DIR == it->second.type)
			{
#ifdef WIN32
				mkdir(full_path.c_str());
#else
				mkdir(full_path.c_str(), S_IRWXU);
#endif
			}
			else
			{
				update->m_download_file = fopen(full_path.c_str(), "wb");
				if (NULL == update->m_download_file)
				{
					curl_easy_cleanup(curl);
					return NULL;
				}
				curl_easy_setopt(curl, CURLOPT_URL, (update->m_server_update_url + "/update/" + it->first).c_str());
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteDownload);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, update);
				if (0 != curl_easy_perform(curl))
				{
					curl_easy_cleanup(curl);
					fclose(update->m_download_file);
					return NULL;
				}
				fclose(update->m_download_file);
			}
		}
	}

	return NULL;
}

void Update::update()
{
	//assert(HasNewVersion())
	for (UpdateMap::iterator it = m_server_update_map.begin(); it != m_server_update_map.end(); ++it)
	{
		UpdateMap::iterator find_it = m_local_update_map.find(it->first);
		if (find_it != m_local_update_map.end())
		{
			if (0 == memcmp(find_it->second.md5, it->second.md5, 32))
			{
				find_it->second.opt_type = UpdateFile::OT_KEEP;
			}
			else
			{
				find_it->second.opt_type = UpdateFile::OT_UPDATE;
				m_update_size += it->second.size;
			}
		}
		else
		{
			it->second.opt_type =UpdateFile::OT_CREATE;
			m_local_update_map.insert(make_pair(it->first, it->second));
			m_update_size += it->second.size;
		}
	}
	pthread_create(m_download_thread, NULL, UpdateThread, this);
	while (m_download_size < m_update_size)
	{
		printf("download %d / %d\n", m_download_size, m_update_size);
	}
	FILE* update_file = fopen((m_local_update_path + "/update.xml").c_str(), "wb");
	{
		if (NULL != update_file)
		{
			fwrite(m_server_update_str.c_str(), 1, m_server_update_str.length(), update_file);
			fclose(update_file);
			FILE* version_file = fopen((m_local_update_path + "/version.txt").c_str(), "wb");
			if (NULL != version_file)
			{
				fwrite(m_server_version.c_str(), 1, m_server_version.length(), version_file);
				fclose(version_file);
			}
		}
	}
	
	printf("update finished\n");
}


