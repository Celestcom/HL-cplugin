#include "stdafx.h"
#include "Loader.h"
#include "HapticFileInfo.h"
#include <iostream>
#include "Locator.h"

using namespace boost::filesystem;

Loader::Loader(const std::string& basePath): _parser(std::make_shared<Parser>(basePath))
{
	_sequenceLoader = std::make_shared<SequenceLoader>(_parser);
	_patternLoader = std::make_shared<PatternLoader>(_parser, _sequenceLoader);
	_experienceLoader = std::make_shared<ExperienceLoader>(_parser, _patternLoader);

	_parser->EnumerateHapticFiles();
}


Loader::~Loader()
{
}

shared_ptr<PatternLoader> Loader::GetPatternLoader() const
{
	return _patternLoader;
}

shared_ptr<SequenceLoader> Loader::GetSequenceLoader() const
{
	return _sequenceLoader;
}

shared_ptr<ExperienceLoader> Loader::GetExperienceLoader() const
{
	return _experienceLoader;
}

bool Loader::Load(const HapticFileInfo& fileInfo) const
{
	switch (fileInfo.FileType)
	{
	case HapticFileType::Pattern:
		return _patternLoader->Load(fileInfo);
	case HapticFileType::Sequence:
		return _sequenceLoader->Load(fileInfo);
	case HapticFileType::Experience:
		return _experienceLoader->Load(fileInfo);
	}
	return false;
}

SequenceLoader::SequenceLoader(shared_ptr<Parser> p)
	:_parser(p) {
}

SequenceLoader::~SequenceLoader()
{
}

bool SequenceLoader::Load(const HapticFileInfo& fileInfo)
{
	if (_sequences.find(fileInfo.FullId) != _sequences.end()) {
		return true;
	}

	auto validNames = fileInfo.GetValidFileNames();
	try {
		const path directory = _parser->GetDirectory(fileInfo.FullyQualifiedPackage) /= fileInfo.GetDirectory();
		for (auto name : validNames)
		{
			auto dir = directory;
			auto path = dir /= name;
			if (exists(path)) {
				vector<JsonSequenceAtom> sequence = _parser->ParseSequence(path);
				_sequences[fileInfo.FullId] = JsonSequence(fileInfo.FullId, sequence);
				return true;
			}

		}
		std::cout << "Failed to load " << fileInfo.ToString() << ": file not found (in package " << fileInfo.FullyQualifiedPackage << ")\n";

	}
	catch (PackageNotFoundException& p) {
		//package not found
		return false;
	}
	return false;
}

JsonSequence SequenceLoader::GetLoadedResource(const std::string& key)
{
	return _sequences.at(key);
}

PatternLoader::PatternLoader(shared_ptr<Parser> p, shared_ptr<SequenceLoader> seq)
:_sequenceLoader(std::move(seq)), _parser(p){
}

PatternLoader::~PatternLoader()
{
}

bool PatternLoader::Load(const HapticFileInfo& fileInfo)
{
	if (_patterns.find(fileInfo.FullId) != _patterns.end()) {
		return true;
	}

	auto validNames = fileInfo.GetValidFileNames();
	try {
		path directory = _parser->GetDirectory(fileInfo.FullyQualifiedPackage) /= fileInfo.GetDirectory();
		for (auto name : validNames)
		{

			auto dir = directory;
			auto path = dir /= name;
			if (exists(path)) {
				auto atoms = _parser->ParsePattern(path);
				_patterns[fileInfo.FullId] = JsonPattern(fileInfo.FullId, atoms);
				loadAllSequences(atoms);
				return true;
			}

		}
	}
	catch (PackageNotFoundException& p) {
		//package not found
		return false;
	}
	return false;
}

JsonPattern PatternLoader::GetLoadedResource(const std::string & key)
{
	return _patterns.at(key);
}

void PatternLoader::loadAllSequences(vector<JsonPatternAtom> pattern) const
{
	for (auto atom : pattern) {
		SequenceFileInfo info(atom.Sequence);
		_sequenceLoader->Load(info);
	}
	
}

ExperienceLoader::ExperienceLoader(shared_ptr<Parser> p, shared_ptr<PatternLoader> pat)
	:_patternLoader(std::move(pat)), _parser(p)
{
}


ExperienceLoader::~ExperienceLoader()
{
}

bool ExperienceLoader::Load(const HapticFileInfo& fileInfo)
{
	if (_experiences.find(fileInfo.FullId) != _experiences.end()) {
		return true;
	}

	auto validNames = fileInfo.GetValidFileNames();
	try {
		path directory = _parser->GetDirectory(fileInfo.FullyQualifiedPackage) /= fileInfo.GetDirectory();
		for (auto name : validNames)
		{

			auto dir = directory;
			auto path = dir /= name;
			if (exists(path)) {
				auto atoms = _parser->ParseExperience(path);
				_experiences[fileInfo.FullId] = JsonExperience(fileInfo.FullId, atoms);
				loadAllPatterns(atoms);
				return true;
			}

		}
	}
	catch (PackageNotFoundException& p) {
		//package not found
		return false;
	}
	return false;
}

JsonExperience ExperienceLoader::GetLoadedResource(const std::string & key)
{
	return _experiences.at(key);
}



void ExperienceLoader::loadAllPatterns(vector<JsonExperienceAtom> experience) const
{
	for (auto atom : experience) {
		PatternFileInfo info(atom.Pattern);
		_patternLoader->Load(info);
	}
}

