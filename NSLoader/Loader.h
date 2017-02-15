#pragma once
#include <memory>
#include <unordered_map>
#include "Parser.h"
#include "HapticClasses.h"

class Parser;
class HapticFileInfo;
class PatternLoader;
class SequenceLoader;
class ExperienceLoader;

using namespace std;
using namespace NullSpace;
template<class T>
class IHapticLoadingStrategy
{
public:
	virtual ~IHapticLoadingStrategy()
	{
	}
	virtual bool Load(const HapticFileInfo& fileInfo) = 0;
	virtual T GetLoadedResource(const std::string& key) = 0;
};

class NodeSequenceLoader : public IHapticLoadingStrategy<Node> {
public: 
	NodeSequenceLoader(std::shared_ptr<Parser>);
	~NodeSequenceLoader();
	bool Load(const HapticFileInfo& fileInfo) override;
	Node GetLoadedResource(const std::string& key) override;
private:
	std::shared_ptr<Parser> _parser;
	unordered_map<string, Node> _nodes;
};

class NodePatternLoader : public IHapticLoadingStrategy<Node> {
public:
	NodePatternLoader(std::shared_ptr<Parser>, std::shared_ptr<NodeSequenceLoader>);
	~NodePatternLoader();
	bool Load(const HapticFileInfo& fileInfo) override;
	Node GetLoadedResource(const std::string& key) override;
private:
	std::shared_ptr<Parser> _parser;
	std::shared_ptr<NodeSequenceLoader> _sequenceLoader;

	unordered_map<string, Node> _nodes;
	void loadAllSequences(vector<JsonPatternAtom>) const;

};

class NodeExperienceLoader : public IHapticLoadingStrategy<Node> {
public:
	NodeExperienceLoader(std::shared_ptr<Parser>, std::shared_ptr<NodePatternLoader>);
	~NodeExperienceLoader();
	bool Load(const HapticFileInfo& fileInfo) override;
	Node GetLoadedResource(const std::string& key) override;
private:
	std::shared_ptr<Parser> _parser;
	std::shared_ptr<NodePatternLoader> _patternLoader;

	unordered_map<string, Node> _nodes;
	void loadAllNodePatterns(vector<JsonExperienceAtom>) const;

};


class SequenceLoader : public IHapticLoadingStrategy<JsonSequence>
{
public:
	SequenceLoader(std::shared_ptr<Parser>);
	~SequenceLoader();
	bool Load(const HapticFileInfo& fileInfo) override;
	JsonSequence GetLoadedResource(const std::string& key) override;
private:
	std::shared_ptr<Parser> _parser;
	unordered_map<string, JsonSequence> _sequences;
};

class PatternLoader : public IHapticLoadingStrategy<JsonPattern>
{
public:
	PatternLoader(std::shared_ptr<Parser>, std::shared_ptr<SequenceLoader>);
	~PatternLoader();
	bool Load(const HapticFileInfo& fileInfo) override;
	JsonPattern GetLoadedResource(const std::string& key) override;
private:
	std::shared_ptr<SequenceLoader> _sequenceLoader;
	std::shared_ptr<Parser> _parser;
	unordered_map<string, JsonPattern> _patterns;
	void loadAllSequences(vector<JsonPatternAtom>) const;
};

class ExperienceLoader : public IHapticLoadingStrategy<JsonExperience>
{
public:
	ExperienceLoader(std::shared_ptr<Parser>, std::shared_ptr<PatternLoader>);
	~ExperienceLoader();
	bool Load(const HapticFileInfo& fileInfo) override;
	JsonExperience GetLoadedResource(const std::string& key) override;
private:
	std::shared_ptr<PatternLoader> _patternLoader;
	std::shared_ptr<Parser> _parser;
	unordered_map<string, JsonExperience> _experiences;
	void loadAllPatterns(vector<JsonExperienceAtom>) const;
};
class Loader
{
public:
	Loader(const std::string& basePath);
	~Loader();
	std::shared_ptr<PatternLoader> GetPatternLoader() const;
	std::shared_ptr<SequenceLoader> GetSequenceLoader() const;
	std::shared_ptr<ExperienceLoader> GetExperienceLoader() const;
	bool Load(const HapticFileInfo& fileInfo) const;

private:
	std::shared_ptr<Parser> _parser;
	std::shared_ptr<PatternLoader> _patternLoader;
	std::shared_ptr<ExperienceLoader> _experienceLoader;
	std::shared_ptr<SequenceLoader> _sequenceLoader;



};

class NodeLoader
{
public:
	NodeLoader(const std::string& basePath);
	~NodeLoader();
	std::shared_ptr<NodePatternLoader> GetPatternLoader() const;
	std::shared_ptr<NodeSequenceLoader> GetSequenceLoader() const;
	std::shared_ptr<NodeExperienceLoader> GetExperienceLoader() const;
	bool Load(const HapticFileInfo& fileInfo) const;

private:
	std::shared_ptr<Parser> _parser;
	std::shared_ptr<NodePatternLoader> _patternLoader;
	std::shared_ptr<NodeExperienceLoader> _experienceLoader;
	std::shared_ptr<NodeSequenceLoader> _sequenceLoader;



};