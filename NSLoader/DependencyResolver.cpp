#include "stdafx.h"
#include "DependencyResolver.h"
#include "Locator.h"
#include <iostream>
#include "HapticFileInfo.h"
#include "AreaParser.h"
class HapticsNotLoadedException : public std::runtime_error {
public:
	HapticsNotLoadedException(const HapticArgs& args) : std::runtime_error(std::string("Attempted to resolve " + args.ToString() + ", but it was not loaded so failed.").c_str()) {}

};



NodeDependencyResolver::NodeDependencyResolver(const std::string & basePath) :_loader(basePath)
{
	
	
}



NodeDependencyResolver::~NodeDependencyResolver()
{
}

Node NodeDependencyResolver::Resolve(const SequenceArgs & args)
{
	Node root = _loader.GetSequenceLoader()->GetLoadedResource(args.Name);
	root.Area = (uint32_t)args.Location;
	root.Strength = args.Strength;

	
	return root;
}

Node NodeDependencyResolver::Resolve(const PatternArgs & args)
{
	Node root = _loader.GetPatternLoader()->GetLoadedResource(args.Name);
	Node packedRoot(Node::EffectType::Pattern);
	packedRoot.Effect = root.Effect;

	for (auto n : root.Children) {
		Node resolved = Resolve(SequenceArgs(n.Effect, (AreaFlag)n.Area, n.Strength));
		resolved.Time = n.Time;
		packedRoot.Children.push_back(resolved);
	}

	packedRoot.Strength = root.Strength;

	return packedRoot;
}

Node NodeDependencyResolver::Resolve(const ExperienceArgs & args)
{
	Node root = _loader.GetExperienceLoader()->GetLoadedResource(args.Name);
	Node packedRoot(Node::EffectType::Experience);
	packedRoot.Effect = root.Effect;
	for (auto n : root.Children) {
		Node resolved = Resolve(PatternArgs(n.Effect, Side::NotSpecified));
		resolved.Time = n.Time;
		packedRoot.Children.push_back(resolved);
	}
	packedRoot.Strength = root.Strength;
	return packedRoot;
}

std::vector<Node*> NodeDependencyResolver::Flatten(Node * rootNode)
{
	rootNode->Propogate(0.0, 1.0, (uint32_t)AreaFlag::None);
	std::vector<Node*> result;
	Visit(rootNode, result);
	std::sort(result.begin(), result.end(), [](const Node* lhs, const Node* rhs) {return lhs->Time < rhs->Time; });
	return result;
}

bool NodeDependencyResolver::Load(const HapticFileInfo & info)
{
	return _loader.Load(info);
}

void NodeDependencyResolver::Visit(Node * node, std::vector<Node*>& result)
{
	//The commented code is for making sure this is a DAG, Which it is
	//assert(!node->Marked());
	//if (node->Marked()) {
	//	return;
	//}

	if (node->Children.empty()) {
		result.push_back(node);
	//	node->Mark();
		return;
	}

	for (auto& child : node->Children) {
		Visit(&child, result);
	}
}

