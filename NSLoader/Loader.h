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



class SequenceLoader : public IHapticLoadingStrategy<JsonSequence>
{
public:
	SequenceLoader(shared_ptr<Parser>);
	~SequenceLoader();
	bool Load(const HapticFileInfo& fileInfo) override;
	JsonSequence GetLoadedResource(const std::string& key) override;
private:
	shared_ptr<Parser> _parser;
	unordered_map<string, JsonSequence> _sequences;
};

class PatternLoader : public IHapticLoadingStrategy<JsonPattern>
{
public:
	PatternLoader(shared_ptr<Parser>, shared_ptr<SequenceLoader>);
	~PatternLoader();
	bool Load(const HapticFileInfo& fileInfo) override;
	JsonPattern GetLoadedResource(const std::string& key) override;
private:
	shared_ptr<SequenceLoader> _sequenceLoader;
	shared_ptr<Parser> _parser;
	unordered_map<string, JsonPattern> _patterns;
	void loadAllSequences(vector<JsonPatternAtom>) const;
};

class ExperienceLoader : public IHapticLoadingStrategy<JsonExperience>
{
public:
	ExperienceLoader(shared_ptr<Parser>, shared_ptr<PatternLoader>);
	~ExperienceLoader();
	bool Load(const HapticFileInfo& fileInfo) override;
	JsonExperience GetLoadedResource(const std::string& key) override;
private:
	shared_ptr<PatternLoader> _patternLoader;
	shared_ptr<Parser> _parser;
	unordered_map<string, JsonExperience> _experiences;
	void loadAllPatterns(vector<JsonExperienceAtom>) const;
};
class Loader
{
public:
	Loader(const std::string& basePath);
	~Loader();
	shared_ptr<PatternLoader> GetPatternLoader() const;
	shared_ptr<SequenceLoader> GetSequenceLoader() const;
	shared_ptr<ExperienceLoader> GetExperienceLoader() const;
	bool Load(const HapticFileInfo& fileInfo) const;

private:
	shared_ptr<Parser> _parser;
	shared_ptr<PatternLoader> _patternLoader;
	shared_ptr<ExperienceLoader> _experienceLoader;
	shared_ptr<SequenceLoader> _sequenceLoader;



};