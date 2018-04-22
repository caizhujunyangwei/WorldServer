
#include <curl/curl.h>
#include <functional>

extern std::string NULL_STRING;

size_t getData(void* data, size_t i1, size_t i2, void* pNode);

enum ReadyState{
	UNSET = 0,
	SETTING = 1,
	SEND = 2,
	GET = 3
};

class HttpRequest
{
public:
	HttpRequest();
	~HttpRequest();

	std::function<void (void*,size_t)> onreadystatechange;
	std::function<void(void*)> onloadstart;
	std::function<void(void*)> onload;
	std::function<void(void*)> onloadend;
	std::function<void(void*)> onabort;
	std::function<void(void*)> onerror;
	std::function<void(void*)> ontimeout;

	int m_readystate;
	int m_responseCode;
	void* m_responseData;
	int m_responseDataSize;

	void* m_targetData;
protected:
	friend size_t getData(void* data, size_t i1, size_t i2, void* pNode);
private:
	std::string m_url;
	std::string m_method;

	CURL* curl;
public:
	bool open(const std::string method, const std::string url);
	bool send();

	bool setTargetData(void* data) { m_targetData = data; };
};


#define setreadystatechange(_phttp,_pfunc,_target)	\
{\
	do\
	{\
		_phttp->onreadystatechange = std::bind(_pfunc,_target,std::placeholders::_1,std::placeholders::_2);\
	}while (0); \
}

#define setonloadstart(_phttp,_pfunc,_target)\
{\
	do\
	{\
		_phttp->onloadstart = std::bind(_pfunc, _target, std::placeholders::_1); \
	}while (0); \
}

#define setonload(_phttp,_pfunc,_target)\
{\
	do\
	{\
		_phttp->onload = std::bind(_pfunc, _target, std::placeholders::_1); \
	}while (0); \
}

#define setonloadend(_phttp,_pfunc,_target)\
{\
	do\
	{\
		_phttp->onloadend = std::bind(_pfunc, _target, std::placeholders::_1); \
	}while (0); \
}

#define setonabort(_phttp,_pfunc,_target)\
{\
	do\
	{\
		_phttp->onabort = std::bind(_pfunc, _target, std::placeholders::_1); \
	}while (0); \
}

#define setonerror(_phttp,_pfunc,_target)\
{\
	do\
	{\
		_phttp->onerror = std::bind(_pfunc, _target, std::placeholders::_1); \
	}while (0); \
}

#define setontimeout(_phttp,_pfunc,_target)\
{\
	do\
	{\
		_phttp->ontimeout = std::bind(_pfunc, _target, std::placeholders::_1); \
	}while (0); \
}