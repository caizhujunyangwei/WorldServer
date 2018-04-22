#include "Module.h"

NS_MODULE_BEGINE

Module::Module()
	:tag(0),runflag(false)
{
}

Module::~Module()
{
	this->setRunFlag(false);
	this->setTag(0);
}

bool Module::init()
{
	this->setRunFlag(false);
	this->setTag(0);
	return true;
}

bool Module::run(bool)
{
	this->setRunFlag(true);
	return true;
}

bool Module::stop()
{
	if (this->runflag)
	{
		setRunFlag(false);
		//delete (this);
	}
	else
		return false;

	return true;
}

NS_MODULE_END
