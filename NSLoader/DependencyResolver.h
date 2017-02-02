#pragma once
#include <vector>
#include "HapticArgs.h"
#include "HapticCache.h"
#include "Parser.h"
#include "HapticClasses.h"
#include <memory>
#include "Loader.h"
using namespace std;


template<class TArgType, class T>
class IResolvable
{
public:
	virtual ~IResolvable()
	{
	}

	virtual T Resolve(TArgType args) = 0;
};


class NodeDependencyResolver {
public:
	NodeDependencyResolver(const std::string& basePath);
	~NodeDependencyResolver();
	Node Resolve(const SequenceArgs& args);
	Node Resolve(const PatternArgs& args);
	Node Resolve(const ExperienceArgs& args);
	std::vector<Node*> Flatten(Node* rootNode);

	bool Load(const HapticFileInfo& info);
	
private:
	
	NodeLoader _loader;

	void Visit(Node* node, std::vector<Node*>& result);
};

