#ifndef __LOG_H__
#define	__LOG_H__
/**************************************************************************************
 * FILE:log.h
 * DATE:2017/12/11
 * AUTH:YangW
 * INTR:Log基类
**************************************************************************************/

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <execinfo.h>
#include <stdlib.h>
#include "../Module/ModuleLock.h"
#include "../Module/ModuleTime.h"

#define MSG_LENGTH		1024 * 5

US_NS_MODULE;

enum LEVEL
{
	Info   = 1,
	Warn   = 2,
	Debug  = 3,
	Error  = 4
};

class Log
{
	~Log();
private:
	Log();
	static Log* instance;
	AALock* m_logLock;
	//pthread_mutex_t* _mutex;
	char* message;
	
private:
	void Log_Message(LEVEL level);

public:
	static Log* getInstance();
	
	static void Debug(const char* format,...);
	static void Info(const char* format,...);
	static void Warn(const char* format,...);
	static void Error(const char* format,...);
};


#endif //__LOG_H__