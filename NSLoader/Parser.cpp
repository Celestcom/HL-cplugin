#include "stdafx.h"
#include "Parser.h"
#include "json\json.h"


std::string GetFileType(HapticFileType ftype)
{
	switch (ftype)
	{
	case HapticFileType::Experience:
		return "experience";
	case HapticFileType::Sequence:
		return "sequence";
	case HapticFileType::Pattern:
		return "pattern";
	default:
		return "unrecognized";
	}
}

Parser::Parser()
{
	
}

Parser::Parser(const std::string& path) : 
	_basePath(path)
{
}

Parser::~Parser()
{
}


void Parser::SetBasePath(const std::string& path)
{
	_basePath = path;
}

void Parser::EnumerateHapticFiles()
{
	_paths.clear();
	HapticEnumerator enumerator(_basePath);
	EnumNode root = enumerator.GenerateNodes(enumerator.GetEnums(enumerator.EnumerateFiles()));
	for (auto child : root.Children)
	{
		Traverse(child.second, root.Namespace);
	}
}

boost::filesystem::path Parser::GetDirectory(std::string package) {
	if (_paths.find(package) != _paths.end())
	{
		return _paths[package];
	}
	throw PackageNotFoundException(package);
}
void Parser::Traverse(EnumNode node, std::string prefix)
{
	//if it has a namespace,
	//then check if it had a parent namespace. 
	if (node.Data.Namespace!= "")
	{
		if (prefix == "")
		{
			_paths[node.Namespace] = node.Data.Directory;
		} else
		{
			_paths[prefix + "." + node.Namespace] = node.Data.Directory;
		}
	}

	if (node.Children.size() == 0)
	{
		return;
	}

	for (auto child : node.Children)
	{
		//if PARENT is a toplevel node, we want to pass in just the parent name as the prefix
		//if PARENT is a child node, we want to pass in the full history by adding it on
		Traverse(child.second, prefix == "" ? node.Namespace : prefix + "." + node.Namespace);
	}
	
}

std::vector<JsonSequenceAtom> Parser::ParseSequence(boost::filesystem::path path)
{
	std::vector<JsonSequenceAtom> outItems;
	//check if exists
	Json::Value root;
	std::ifstream json(path.string(), std::ifstream::binary);
	json >> root;
	if (root.isMember("sequence") && root["sequence"].isArray())
	{
		for (auto x : root["sequence"])
		{
			JsonSequenceAtom s;
			s.Deserialize(x);
			outItems.push_back(s);
		}
	}
	else {
		throw MalformedHapticsFileException("key 'sequence' must be present in sequence files");
	}
	return outItems;
}

std::vector<JsonPatternAtom> Parser::ParsePattern(boost::filesystem::path path)
{
	std::vector<JsonPatternAtom> outAtoms;
	//check if exists
	Json::Value root;
	std::ifstream json(path.string(), std::ifstream::binary);
	json >> root;
	if (root.isMember("pattern") && root["pattern"].isArray())
	{
		for (auto x : root["pattern"])
		{
			JsonPatternAtom f;
			f.Deserialize(x);
			outAtoms.push_back(f);
		}
	}
	else {
		throw MalformedHapticsFileException( "key 'pattern' must be present in pattern files");
	}
	return outAtoms;
}
std::vector<JsonExperienceAtom> Parser::ParseExperience(boost::filesystem::path path)
{
	std::vector<JsonExperienceAtom> outAtoms;
	//check if exists
	Json::Value root;
	std::ifstream json(path.string(), std::ifstream::binary);
	json >> root;
	if (root.isMember("experience") && root["experience"].isArray())
	{
		for (auto x : root["experience"])
		{
			JsonExperienceAtom f;
			f.Deserialize(x);
			outAtoms.push_back(f);
		}
	}
	else {
		throw MalformedHapticsFileException( "key 'experience' must be present in experience files");
	}
	return outAtoms;
	
}
