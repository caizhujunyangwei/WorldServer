#include "PublicCommon.h"
#include <curl/curl.h>
#include "../Utils/Log.h"

void getJsonFromWeb(const char* url, std::map<const char*,const char*>* mapData,
	httpBack callback,std::vector<const char*>* vecHeaders)
{
	//easy handler的句柄  
	CURL* curl = nullptr;
	CURLcode res = CURLE_OK;

	//HTTP报文头  
	struct curl_slist* headers = NULL;
	if(vecHeaders)
		for(int j = 0;j < (*vecHeaders).size();++j)
		{
			headers = curl_slist_append(headers, (*vecHeaders)[j]);
		}

	//URL拼接
	std::string urlStr(url);
	char code = '?';
	for (std::map<const char*, const char*>::iterator it = mapData->begin(); it != mapData->end();++it)
	{
		urlStr += code;

		//const char* s = it->second;
		urlStr += it->first;
		urlStr += '=';
		urlStr += it->second;

		code = '&';
	}

	Log::Error("real URL is :%s ", urlStr.c_str());

	curl = curl_easy_init();
	if (curl)
	{
		//设置post请求的url地址  
		curl_easy_setopt(curl, CURLOPT_URL, urlStr.c_str());
		//设置发送超时时间  
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0.5);
		//设置HTTP头  
		//curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		//设置数据接收回调函数
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);

		//执行单条请求  
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			Log::Error("send httpRequest %s failed [%s]", url, curl_easy_strerror(res));
		}

		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
	}
	curl = nullptr;
	res = CURLE_OK;
}