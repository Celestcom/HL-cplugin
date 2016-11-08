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
class DependencyResolver
{
public:
	DependencyResolver(const std::string& basePath);
	~DependencyResolver();
	//void SetBasePath(const std::string& path);
	static Location ComputeLocationSide(JsonLocation loc, Side side);
	PackedSequence ResolveSequence(const std::string& name, std::string location) const;
	PackedPattern ResolvePattern(const std::string& name, Side side) const;
	std::vector<HapticSample> ResolveSample(const std::string& name, Side side) const;


	bool Load(const HapticFileInfo& fileInfo) const;
private:
	shared_ptr<IResolvable<SequenceArgs, PackedSequence>> _sequenceResolver;
	shared_ptr<IResolvable<PatternArgs, PackedPattern>> _patternResolver;
	shared_ptr<IResolvable<ExperienceArgs, std::vector<HapticSample>>> _experienceResolver;
	Loader _loader;
	
};




class SequenceResolver : public IResolvable<SequenceArgs, PackedSequence>
{
public:
	SequenceResolver(shared_ptr<SequenceLoader> loader);
	~SequenceResolver();
	PackedSequence Resolve(SequenceArgs args) override;
private:
	HapticCache<PackedSequence> _cache;
	shared_ptr<SequenceLoader> _sequenceLoader;
	static HapticEffect transformSequenceItemIntoEffect(const JsonSequenceAtom& seq, Location loc);
};

class PatternResolver : public IResolvable<PatternArgs, PackedPattern>
{
public:
	PatternResolver(shared_ptr<IResolvable<SequenceArgs, PackedSequence>>  seq, shared_ptr<PatternLoader>);
	~PatternResolver();
	PackedPattern Resolve(PatternArgs args) override;
private:
	HapticCache<PackedPattern> _cache;
	shared_ptr<PatternLoader> _patternLoader;
	shared_ptr<IResolvable<SequenceArgs, PackedSequence>>  _seqResolver;
	HapticFrame transformFrameToHapticFrame(const Frame& frame, Side side) const;
	static Side ComputeSidePrecedence(Side inputSide, Side programmaticSide);
};

class ExperienceResolver : public IResolvable<ExperienceArgs, std::vector<HapticSample>>
{
public:
	ExperienceResolver(shared_ptr<IResolvable<PatternArgs, PackedPattern>> pat, shared_ptr<ExperienceLoader> el);
	~ExperienceResolver();
	vector<HapticSample> Resolve(ExperienceArgs args) override;
private:
	HapticCache<std::vector<HapticSample>> _cache;
	shared_ptr<ExperienceLoader> _experienceLoader;
	shared_ptr<IResolvable<PatternArgs, PackedPattern>> _patResolver;
	HapticSample transformMomentToHapticSample(Moment moment, Side side) const;
};
