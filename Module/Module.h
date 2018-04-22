#ifndef __MODULE_H__
#define __MODULE_H__
/**************************************************************************************
 * FILE:module.h
 * DATE:2017/12/11
 * AUTH:YangW
 * INTR:通用模块基类
**************************************************************************************/

#include "../Common/PublicCommon.h"

#define NS_MODULE_BEGINE				namespace moduleTask {				
#define NS_MODULE_END					}

#define US_NS_MODULE					using namespace moduleTask

NS_MODULE_BEGINE

class Module
{
	SYNTHESIZE(bool, runflag, RunFlag);				//运行标志
	SYNTHESIZE(unsigned int, tag, Tag);				//模块标签
public:
	Module();
	~Module();  //非虚函数

	virtual bool run(bool is = true);
	virtual bool stop();
	virtual bool init();
};

NS_MODULE_END
#endif//!__MODULE_H__
