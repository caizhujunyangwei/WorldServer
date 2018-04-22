#include "./XMLHttpRequest.h"
#include "./Log.h"

std::string NULL_STRING = std::string();

HttpRequest::HttpRequest() :
	onreadystatechange(nullptr),
	onloadstart(nullptr),
	onload(nullptr),
	onloadend(nullptr),
	onabort(nullptr),
	onerror(nullptr),
	ontimeout(nullptr),
	m_readystate(ReadyState::UNSET),
	m_responseCode(0),
	m_responseData(nullptr),
	curl(nullptr),
	m_responseDataSize(0),
	m_targetData(nullptr)
{
	curl = curl_easy_init();
}

HttpRequest::~HttpRequest()
{

}

bool HttpRequest::open(const std::string method, const std::string url)
{
	m_url = url;
	m_method = method;

	curl_easy_setopt(curl, CURLOPT_URL, m_url.c_str());
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0.5);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getData);
	//设置回调函数的参数，见getUserInfo函数的最后一个参数
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

	m_readystate = ReadyState::SETTING;
	if (this->onreadystatechange) {
		this->onreadystatechange(this,0);
	}

	return true;
}

bool HttpRequest::send()
{
	Log::Info("HttpRequest send :%s", m_url.c_str());
	CURLcode res = curl_easy_perform(curl);

	if (res != CURLE_OK) {
		Log::Error("send httpRequest %s failed [%s]", m_url.c_str(), curl_easy_strerror(res));
		return false;
	}
	//curl_easy_cleanup(curl);
	m_readystate = ReadyState::SEND;
	if (this->onreadystatechange) {
		this->onreadystatechange(this, 0);
	}

	curl_easy_cleanup(curl);
	curl = nullptr;

	return true;
}


size_t getData(void* data, size_t i1, size_t i2, void* pNode)
{
	if (!data) return 0;

	HttpRequest* http = (HttpRequest*)pNode;
	http->m_readystate = ReadyState::GET;
	http->m_responseData = data;
	http->m_responseDataSize = i1*i2;

	if (http->onreadystatechange) {
		http->onreadystatechange(http, i1*i2);
	}


	//
	return (i1*i2);
}