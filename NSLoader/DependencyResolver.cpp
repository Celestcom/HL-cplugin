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



DependencyResolver::DependencyResolver(const std::string& basePath): _loader(basePath)
{
	_sequenceResolver = make_shared<SequenceResolver>(_loader.GetSequenceLoader());
	_patternResolver = make_shared<PatternResolver>(_sequenceResolver, _loader.GetPatternLoader());
	_experienceResolver = make_shared<ExperienceResolver>(_patternResolver, _loader.GetExperienceLoader());


}

DependencyResolver::~DependencyResolver()
{
}



Location DependencyResolver::ComputeLocationSide(JsonLocation location, Side side)
{

	switch (location)
	{
	case JsonLocation::Shoulder:
		return side == Side::Left ? Location::Shoulder_Left : Location::Shoulder_Right;
	case JsonLocation::Upper_Back:
		return side == Side::Left ? Location::Upper_Back_Left : Location::Upper_Back_Right;
	case JsonLocation::Lower_Ab:
		return side == Side::Left ? Location::Lower_Ab_Left : Location::Lower_Ab_Right;
	case JsonLocation::Mid_Ab:
		return side == Side::Left ? Location::Mid_Ab_Left : Location::Mid_Ab_Right;
	case JsonLocation::Upper_Ab:
		return side == Side::Left ? Location::Upper_Ab_Left : Location::Upper_Ab_Right;
	case JsonLocation::Chest:
		return side == Side::Left ? Location::Chest_Left : Location::Chest_Right;
	case JsonLocation::Upper_Arm:
		return side == Side::Left ? Location::Upper_Arm_Left : Location::Upper_Arm_Right;
	case JsonLocation::Forearm:
		return side == Side::Left ? Location::Forearm_Left : Location::Forearm_Right;
	default:
		return Location::Error;
	}
}

PackedSequence DependencyResolver::ResolveSequence(const std::string & name, AreaFlag location) const
{
	return _sequenceResolver->Resolve(SequenceArgs(name, location));
}

PackedPattern DependencyResolver::ResolvePattern(const std::string& name) const
{
	//Todo: replace side with ops
	return _patternResolver->Resolve(PatternArgs(name, Side::NotSpecified));
}

PackedExperience DependencyResolver::ResolveExperience(const std::string& name) const
{
	return _experienceResolver->Resolve(ExperienceArgs(name, Side::NotSpecified));
}

std::vector<JsonSequenceAtom> DependencyResolver::FetchCachedSequence(const std::string & name) const
{
	return _loader.GetSequenceLoader()->GetLoadedResource(name).JsonAtoms();
}

bool DependencyResolver::Load(const HapticFileInfo& fileInfo) const
{
	return _loader.Load(fileInfo);
}


SequenceResolver::~SequenceResolver()
{
}

SequenceResolver::SequenceResolver(shared_ptr<SequenceLoader> loader) :_sequenceLoader(loader)
{
}

PackedSequence SequenceResolver::Resolve(SequenceArgs args)
{
	//if (_loadedFiles.find(args.Name) != _loadedFiles.end())
	//{
	//	throw HapticsNotLoadedException(args);
	//}
	
	if (_cache.Contains(args)) {
		return _cache.Get(args);
	}
	auto inputItems = _sequenceLoader->GetLoadedResource(args.Name);

	auto packed = PackedSequence(inputItems.Name(), inputItems.JsonAtoms(), args.Location);
	_cache.Cache(args, packed);
	return packed;
}

HapticEffect SequenceResolver::transformSequenceItemIntoEffect(const JsonSequenceAtom& seq, Location loc)
{
	//TODO: priority and explain default values
	Effect effect = Locator::getTranslator().ToEffect(seq.Effect, Effect::Buzz_100);
	return HapticEffect(effect, loc, seq.Duration, seq.Time, 1);
}

PatternResolver::PatternResolver(shared_ptr<IResolvable<SequenceArgs, PackedSequence>>  seq, shared_ptr<PatternLoader> p)
: _patternLoader(p), _seqResolver(seq)
{
}




PatternResolver::~PatternResolver()
{
}

PackedPattern PatternResolver::Resolve(PatternArgs args)
{
	if (_cache.Contains(args)) {
		return _cache.Get(args);
	}

	std::vector<TimeIndex<PackedSequence>> seqs;
	
	auto jsonpat = _patternLoader->GetLoadedResource(args.Name);
	seqs.reserve(jsonpat.JsonAtoms().size());

	for (auto seq : jsonpat.JsonAtoms()) {
		auto parsedArea = AreaParser(seq.Area).GetArea();
		seqs.push_back(TimeIndex<PackedSequence>(
			seq.Time, 
			_seqResolver->Resolve(SequenceArgs(seq.Sequence, parsedArea))));
	}
	auto packed = PackedPattern(args.Name, seqs);
	_cache.Cache(args, packed);
	return packed;
}


Side PatternResolver::ComputeSidePrecedence(Side inputSide, Side programmaticSide)
{
	switch (inputSide)
	{
	case Side::NotSpecified:
		return programmaticSide;
	case Side::Inherit:
		return programmaticSide == Side::Inherit ? Side::Mirror : programmaticSide;
	default:
		return inputSide;
	}
}

ExperienceResolver::ExperienceResolver(shared_ptr<IResolvable<PatternArgs, PackedPattern>> pat, shared_ptr<ExperienceLoader> el):
	_experienceLoader(el), _patResolver(pat)
{
}

ExperienceResolver::~ExperienceResolver()
{
}

PackedExperience ExperienceResolver::Resolve(ExperienceArgs args)
{
	//if (_loadedFiles.find(args.Name) != _loadedFiles.end())
	//{
	//	throw HapticsNotLoadedException(args);
	//}
	if (_cache.Contains(args)) {
		return _cache.Get(args);
	}

	std::vector<TimeIndex<PackedPattern>> patterns;

	auto jsonexp = _experienceLoader->GetLoadedResource(args.Name);
	patterns.reserve(jsonexp.JsonAtoms().size());

	for (auto pat : jsonexp.JsonAtoms()) {
		patterns.push_back(TimeIndex<PackedPattern>(
			pat.Time,
			_patResolver->Resolve(PatternArgs(pat.Pattern, Side::NotSpecified))));
	}

	auto packed= PackedExperience(args.Name, patterns);
	_cache.Cache(args, packed);
	return packed;
}
