#pragma once
#include <string>
#include "DependencyResolver.h"
class TestClass
{
private:
	DependencyResolver _resolver;
public:
	TestClass(LPSTR param);
	~TestClass();
	int Play(LPSTR param);
};

