#include "ModuleTime.h"
#include <stdio.h>

NS_MODULE_BEGINE

ModuleTime* ModuleTime::instance = nullptr;
ModuleTime::~ModuleTime()
{
}

ModuleTime * ModuleTime::getInstance()
{
	if (!instance)
	{
		instance = new ModuleTime;
		instance->init();
	}
	return instance;
}

bool ModuleTime::stop()
{
	RETURN_FALSE_IF(!Module::stop());
	delete this;
	return true;
}

time_t ModuleTime::getLocalTimeNumber()
{
	time_t timer;//time_t就是long int 类型
	time(&timer);
	return timer;
}

tm ModuleTime::getLocalTime()
{
	time_t timer;//time_t就是long int 类型
	struct tm *tblock;
	time(&timer);
	tblock = localtime(&timer);
	
	return (*tblock);
}

const char * ModuleTime::convert2ASCTime(tm & t)
{
	return (asctime(&t));
}

void ModuleTime::getDiffTimeStart(timeval & start)
{
	gettimeofday(&start,0);
}

int ModuleTime::getDiffTimeEnd(timeval& start,timeval & end, timeval & diff)
{
	gettimeofday(&end, 0);
	diff.tv_sec = end.tv_sec - start.tv_sec;
	diff.tv_usec = end.tv_usec - start.tv_usec;
	return diff.tv_usec;
}

const char * ModuleTime::convert2TimeStr(timeval & t,char* str)
{

	time_t sec = t.tv_sec;
	//time_t usec = t.tv_usec;
	int h = sec / 3600;
	int m = (sec - h * 3600) / 60;
	int s = sec - h * 3600 - m * 60;
	sprintf(str, "%d:%0.2d:%0.2d", h, m, s);
	
	return str;
}

const long ModuleTime::convertMicro2Milli(long microSeconds)
{
	//1s = 1000ms
	//1s = 1000000 micros
	//1ms = 1000 micros
	return microSeconds / 1000;
}

NS_MODULE_END