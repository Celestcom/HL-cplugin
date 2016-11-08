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
	return outAtoms;
}

std::vector<Sample> Parser::ParseExperience(boost::filesystem::path path)
{
	std::vector<Sample> outSamples;
	//check if exists
	Json::Value root;
	std::ifstream json(path.string(), std::ifstream::binary);
	json >> root;
	if (root.isMember("experience") && root["experience"].isArray())
	{
		for (auto x : root["experience"])
		{
			Sample s;
			s.Deserialize(x);
			outSamples.push_back(s);
		}
	}
	return outSamples;
	
}

void JsonSequenceAtom::Serialize(const Json::Value& root) {

}

void JsonSequenceAtom::Deserialize(const Json::Value& root) {
	this->Time = root.get("time", 0.0f).asFloat();
	this->Effect = root.get("effect", "INVALID_EFFECT").asString();
	this->Strength = root.get("strength", 1.0).asFloat();
	this->Duration = root.get("duration", 0.0f).asFloat();
	this->Repeat = root.get("repeat", 1).asInt();
	
}



void JEffect::Serialize(const Json::Value& root) {

}

void JEffect::Deserialize(const Json::Value& root) {
	this->Location = root.get("location", "INVALID_LOCATION").asString();
	this->Sequence = root.get("sequence", "INVALID_SEQUENCE").asString();
	this->Side = root.get("side", "INVALID_SIDE").asString();

}

void Frame::Serialize(const Json::Value& root) {

}

void Frame::Deserialize(const Json::Value& root) {
	this->Time = root.get("time", 0.0f).asFloat();
	Json::Value frames = root["frame"];
	if (frames.isArray()) {
		for (auto effect : frames) {
			JEffect e;
			e.Deserialize(effect);
			this->FrameSet.push_back(e);
		}
	}
}

void Pattern::Serialize(const Json::Value& root) {

}

Sample::Sample()
{
}

Sample::~Sample()
{
}

void Sample::Deserialize(const Json::Value& root)
{
	Side = root.get("side", "INVALID_SIDE").asString();
	Pattern = root.get("pattern", "INVALID_PATTERN").asString();
	Time = root.get("time", 0.0).asFloat();
	Repeat = root.get("repeat", 0).asUInt();
}

void Sample::Serialize(const Json::Value& root)
{
}

void Pattern::Deserialize(const Json::Value& root) {
	Json::Value frames = root["pattern"];
	if (frames.isArray()) {
		for (auto frame : frames) {
			Frame f;
			f.Deserialize(frame);
			this->Frames.push_back(f);
		}
	}
}

void JsonPatternAtom::Deserialize(const Json::Value & root)
{
	this->Time = root.get("time", 0.0).asFloat();
	this->Sequence = root.get("sequence", "UNKNOWN_SEQUENCE").asString();
	this->Area = root.get("area", "UNKNOWN_AREA").asString();
	
}
