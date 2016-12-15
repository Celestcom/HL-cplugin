#include "stdafx.h"
#include "Loader.h"
#include "HapticFileInfo.h"
#include <iostream>
#include "Locator.h"
#include "AreaParser.h"
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

	const path directory = _parser->GetDirectory(fileInfo.FullyQualifiedPackage) /= fileInfo.GetDirectory();
	for (auto name : validNames)
	{
		auto dir = directory;
		auto path = dir /= name;
		if (exists(path)) {
			try {
				vector<JsonSequenceAtom> sequence = _parser->ParseSequence(path);
				_sequences[fileInfo.FullId] = JsonSequence(fileInfo.FullId, sequence);
				return true;
			}
			catch (const MalformedHapticsFileException& e) {
				throw MalformedHapticsFileException(fileInfo.FullId, e.what());
			}
		}

	}
	throw FileNotFoundException(fileInfo.ToString(), fileInfo.FullyQualifiedPackage);
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

	path directory = _parser->GetDirectory(fileInfo.FullyQualifiedPackage) /= fileInfo.GetDirectory();
	for (auto name : validNames)
	{

		auto dir = directory;
		auto path = dir /= name;
		if (exists(path)) {
			try {
				auto atoms = _parser->ParsePattern(path);
				_patterns[fileInfo.FullId] = JsonPattern(fileInfo.FullId, atoms);
				loadAllSequences(atoms);
				return true;
			}
			catch (const MalformedHapticsFileException& e) {
				throw MalformedHapticsFileException(fileInfo.FullId, e.what());
			}
		
		}

	}
	throw FileNotFoundException(fileInfo.ToString(), fileInfo.FullyQualifiedPackage);

	
}

JsonPattern PatternLoader::GetLoadedResource(const std::string & key)
{
	return _patterns.at(key);
}

void PatternLoader::loadAllSequences(vector<JsonPatternAtom> pattern) const
{
	for (auto atom : pattern) {
		try {
			SequenceFileInfo info(atom.Sequence);
			_sequenceLoader->Load(info);
		}
		catch (const Json::Exception& e) {
			throw Json::Exception(atom.Sequence + ": " + e.what());
		}
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
		path directory = _parser->GetDirectory(fileInfo.FullyQualifiedPackage) /= fileInfo.GetDirectory();
		for (auto name : validNames)
		{

			auto dir = directory;
			auto path = dir /= name;
			if (exists(path)) {
				try {
					auto atoms = _parser->ParseExperience(path);
					_experiences[fileInfo.FullId] = JsonExperience(fileInfo.FullId, atoms);
					loadAllPatterns(atoms);
					return true;
				}
				catch (const MalformedHapticsFileException& e) {
					throw MalformedHapticsFileException(fileInfo.FullId, e.what());
				}
			}

		}
	

	throw FileNotFoundException(fileInfo.ToString(), fileInfo.FullyQualifiedPackage);
}

JsonExperience ExperienceLoader::GetLoadedResource(const std::string & key)
{
	return _experiences.at(key);
}



void ExperienceLoader::loadAllPatterns(vector<JsonExperienceAtom> experience) const
{
	for (auto atom : experience) {
		try {
			PatternFileInfo info(atom.Pattern);
			_patternLoader->Load(info);
		}
		catch (const Json::Exception& e) {
			throw Json::Exception(atom.Pattern + ": " + e.what());
		}
	}
}

NodeSequenceLoader::NodeSequenceLoader(shared_ptr<Parser> p): _parser(p)
{
}

NodeSequenceLoader::~NodeSequenceLoader()
{
}

bool NodeSequenceLoader::Load(const HapticFileInfo & fileInfo)
{
	if (_nodes.find(fileInfo.FullId) != _nodes.end()) {
		return true;
	}

	auto validNames = fileInfo.GetValidFileNames();

	const path directory = _parser->GetDirectory(fileInfo.FullyQualifiedPackage) /= fileInfo.GetDirectory();
	for (auto name : validNames)
	{
		auto dir = directory;
		auto path = dir /= name;
		if (exists(path)) {
			try {
				vector<JsonSequenceAtom> sequence = _parser->ParseSequence(path);
				std::vector<Node> effects;
				effects.reserve(sequence.size());
				for (auto e : sequence) {
					Node effect(Node::EffectType::Effect);
					effect.Duration = e.Duration;
					effect.Strength = e.Strength;
					effect.Time = e.Time;
					effect.Effect = e.Effect;
					effects.push_back(effect);
				}
				Node rootNode(Node::EffectType::Sequence);
				rootNode.Effect = fileInfo.FullId;
				rootNode.Children = effects;
				_nodes[fileInfo.FullId] = rootNode;
				//_sequences[fileInfo.FullId] = JsonSequence(fileInfo.FullId, sequence);
				return true;
			}
			catch (const MalformedHapticsFileException& e) {
				throw MalformedHapticsFileException(fileInfo.FullId, e.what());
			}
		}

	}
	throw FileNotFoundException(fileInfo.ToString(), fileInfo.FullyQualifiedPackage);
}

Node NodeSequenceLoader::GetLoadedResource(const std::string & key)
{
	return _nodes.at(key);
}

NodePatternLoader::NodePatternLoader(shared_ptr<Parser> p, shared_ptr<NodeSequenceLoader> loader):_parser(p), _sequenceLoader(loader)
{
}

NodePatternLoader::~NodePatternLoader()
{
}

bool NodePatternLoader::Load(const HapticFileInfo & fileInfo)
{
	if (_nodes.find(fileInfo.FullId) != _nodes.end()) {
		return true;
	}

	auto validNames = fileInfo.GetValidFileNames();

	path directory = _parser->GetDirectory(fileInfo.FullyQualifiedPackage) /= fileInfo.GetDirectory();
	for (auto name : validNames)
	{

		auto dir = directory;
		auto path = dir /= name;
		if (exists(path)) {
			try {
				auto atoms = _parser->ParsePattern(path);
				std::vector<Node> nodes;
				nodes.reserve(atoms.size());
				for (auto a : atoms) {
					Node node(Node::EffectType::Sequence);
					node.Area = (uint32_t) AreaParser(a.Area).GetArea();
					node.Strength = a.Strength;
					node.Time = a.Time;
					node.Effect = a.Sequence;
				
					nodes.push_back(node);
				}
				Node rootNode(Node::EffectType::Pattern);
				rootNode.Children = nodes;
				rootNode.Effect = fileInfo.FullId;
				_nodes[fileInfo.FullId] = rootNode;
				loadAllSequences(atoms);
				return true;
			}
			catch (const MalformedHapticsFileException& e) {
				throw MalformedHapticsFileException(fileInfo.FullId, e.what());
			}

		}

	}
	throw FileNotFoundException(fileInfo.ToString(), fileInfo.FullyQualifiedPackage);
}

Node NodePatternLoader::GetLoadedResource(const std::string & key)
{
	return _nodes.at(key);
}

void NodePatternLoader::loadAllSequences(vector<JsonPatternAtom> pattern) const
{
	for (auto atom : pattern) {
		try {
			SequenceFileInfo info(atom.Sequence);
			_sequenceLoader->Load(info);
		}
		catch (const Json::Exception& e) {
			throw Json::Exception(atom.Sequence + ": " + e.what());
		}
	}
}

NodeExperienceLoader::NodeExperienceLoader(shared_ptr<Parser> p, shared_ptr<NodePatternLoader> l):_parser(p), _patternLoader(l)
{
}

NodeExperienceLoader::~NodeExperienceLoader()
{
}

bool NodeExperienceLoader::Load(const HapticFileInfo & fileInfo)
{
	if (_nodes.find(fileInfo.FullId) != _nodes.end()) {
		return true;
	}

	auto validNames = fileInfo.GetValidFileNames();
	path directory = _parser->GetDirectory(fileInfo.FullyQualifiedPackage) /= fileInfo.GetDirectory();
	for (auto name : validNames)
	{

		auto dir = directory;
		auto path = dir /= name;
		if (exists(path)) {
			try {
				auto atoms = _parser->ParseExperience(path);
				std::vector<Node> nodes;
				nodes.reserve(atoms.size());
				for (auto a : atoms) {
					Node node(Node::EffectType::Pattern);
					node.Effect = a.Pattern;
					node.Time = a.Time;

					nodes.push_back(node);
				}
				Node rootNode(Node::EffectType::Experience);
				rootNode.Children = nodes;
				rootNode.Effect = fileInfo.FullId;
				_nodes[fileInfo.FullId] = rootNode;
				loadAllNodePatterns(atoms);
				return true;
			}
			catch (const MalformedHapticsFileException& e) {
				throw MalformedHapticsFileException(fileInfo.FullId, e.what());
			}
		}

	}


	throw FileNotFoundException(fileInfo.ToString(), fileInfo.FullyQualifiedPackage);
}

Node NodeExperienceLoader::GetLoadedResource(const std::string & key)
{
	return _nodes.at(key);
}

void NodeExperienceLoader::loadAllNodePatterns(vector<JsonExperienceAtom> experience) const
{
	for (auto atom : experience) {
		try {
			PatternFileInfo info(atom.Pattern);
			_patternLoader->Load(info);
		}
		catch (const Json::Exception& e) {
			throw Json::Exception(atom.Pattern + ": " + e.what());
		}
	}
}

NodeLoader::NodeLoader(const std::string & basePath) : _parser(std::make_shared<Parser>(basePath))
{
	_sequenceLoader = std::make_shared<NodeSequenceLoader>(_parser);
	_patternLoader = std::make_shared<NodePatternLoader>(_parser, _sequenceLoader);
	_experienceLoader = std::make_shared<NodeExperienceLoader>(_parser, _patternLoader);

	_parser->EnumerateHapticFiles();
}

NodeLoader::~NodeLoader()
{
}

shared_ptr<NodePatternLoader> NodeLoader::GetPatternLoader() const
{
	return _patternLoader;
}

shared_ptr<NodeSequenceLoader> NodeLoader::GetSequenceLoader() const
{
	return _sequenceLoader;
}

shared_ptr<NodeExperienceLoader> NodeLoader::GetExperienceLoader() const
{
	return _experienceLoader;
}

bool NodeLoader::Load(const HapticFileInfo & fileInfo) const
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
