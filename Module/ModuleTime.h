#ifndef __MODULE_TIME_H__
#define __MODULE_TIME_H__
/**************************************************************************************
 * FILE:moduletime.h
 * DATE:2017/12/18
 * AUTH:YangW
 * INTR:获取时间和时间相关处理的类 (单例类)
**************************************************************************************/

#include <time.h>  
#include <sys/time.h>   
#include "./Module.h"

NS_MODULE_BEGINE

class ModuleTime :public Module
{
private:
	static ModuleTime* instance;
public:
	~ModuleTime();
	static ModuleTime* getInstance();

	virtual bool stop();
#pragma region 实用函数
	//返回当前时间
	static time_t getLocalTimeNumber();
	//返回当前时间的tm结构指针
	static tm getLocalTime();
	//转化为字符串的事件格式
	static const char* convert2ASCTime(struct tm& t);
	//获取时间差的开始
	static void getDiffTimeStart(struct timeval& start);
	//获取时间差的结束 返回想差的微妙数 diff相差时间结构体
	static int getDiffTimeEnd(struct timeval& start,struct timeval& end, struct timeval& diff);
	//将tm结构转化为时间数值输出
	static const char* convert2TimeStr(struct timeval& t,char* str);
	//将微妙转化为毫秒
	static const long convertMicro2Milli(long microSeconds);
#pragma endregion 实用函数

};

NS_MODULE_END
#endif // !__MODULE_TIME_H__
