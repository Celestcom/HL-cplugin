#pragma once
#include "IJsonSerializable.h"
#include <vector>
#include <unordered_map>

#include "HapticDirectoryTools.h"
#include "Enums.h"
using namespace HapticDirectoryTools;

template<typename T>
struct TimeIndex {
	float Time;
	T Haptic;
	TimeIndex(float time, T haptic) :Time(time), Haptic(haptic) {};
};
class JsonSequenceAtom;
class JsonPatternAtom;
class Frame;
class Sample;

enum class HapticFileType
{
	Pattern = 0,
	Sequence = 1,
	Experience = 2
};

typedef std::string Area;

class PackageNotFoundException : public std::runtime_error {
public:
	PackageNotFoundException(const std::string& package) : std::runtime_error(std::string("The package " + package + " was not found!").c_str()) {}
	
};

std::string GetFileType(HapticFileType ftype);

class Parser
{
public:
	Parser();
	Parser(const std::string& basePath);
	~Parser();
	void SetBasePath(const std::string& basePath);
	void EnumerateHapticFiles();
	boost::filesystem::path GetDirectory(std::string package);
	void Traverse(EnumNode node, std::string prefix);
	std::vector<JsonSequenceAtom> ParseSequence(boost::filesystem::path);
	std::vector<JsonPatternAtom> ParsePattern(boost::filesystem::path);
	std::vector<Sample> ParseExperience(boost::filesystem::path);
	

private:
	std::string _basePath;
	std::unordered_map<std::string, boost::filesystem::path> _paths;

};
class JsonSequenceAtom : public IJsonSerializable {
public:
	float Time;
	std::string Effect;
	float Strength;
	float Duration;
	int Repeat;
	void Deserialize(const Json::Value& root);
	void Serialize(const Json::Value& root);
};
class JsonSequence {
public:
	JsonSequence(std::string name, std::vector<JsonSequenceAtom> atoms) :_name(name), _atoms(atoms) {};
	std::string Name() const { return _name; }
	JsonSequence() = default;
	std::vector<JsonSequenceAtom> JsonAtoms() const { return _atoms; }
private:
	std::string _name;
	std::vector<JsonSequenceAtom> _atoms;
};

class PackedSequence {
public:
	PackedSequence(std::string name, std::vector<JsonSequenceAtom> atoms, Area a) :_name(name), _atoms(atoms), _area(a) {};
	PackedSequence() = default;
	std::string Name() const { return _name; }
	::Area Area() const { return _area; }
	std::vector<JsonSequenceAtom> JsonAtoms() const { return _atoms; }
private:
	std::string _name;
	::Area _area;
	std::vector<JsonSequenceAtom> _atoms;
};

class JsonPatternAtom : public IJsonSerializable {
public:
	float Time;
	std::string Sequence;
	std::string Area;
	void Deserialize(const Json::Value& root);
	void Serialize(const Json::Value& root) {};
};



class PackedPattern {
public:
	PackedPattern(std::string name, std::vector<TimeIndex<PackedSequence>> atoms) : _name(name), _atoms(atoms) {};
	PackedPattern() = default;
	std::string Name() const { return _name; }
	std::vector<TimeIndex<PackedSequence>> PackedAtoms() const { return _atoms; }
	//insert ops here
private:
	std::string _name;
	std::vector<TimeIndex<PackedSequence>> _atoms;
};
class JsonPattern {
public: 
	JsonPattern(std::string name, std::vector<JsonPatternAtom> atoms) : _name(name), _atoms(atoms) {};
	JsonPattern() = default;
	std::string Name() const { return _name; }
	std::vector<JsonPatternAtom> JsonAtoms() const { return _atoms; }
private:
	std::string _name;
	std::vector<JsonPatternAtom> _atoms;
};
class JEffect : public IJsonSerializable {
public:
	std::string Sequence;
	std::string Location;
	std::string Side;
	void Deserialize(const Json::Value& root);
	void Serialize(const Json::Value& root);
};

class Frame : public IJsonSerializable {
public:
	float Time;
	std::vector<JEffect> FrameSet;
	void Deserialize(const Json::Value& root);
	void Serialize(const Json::Value& root);
};


class Pattern : public IJsonSerializable {
public:
	std::vector<Frame> Frames;
	void Deserialize(const Json::Value& root);
	void Serialize(const Json::Value& root);
};



class Sample : public IJsonSerializable
{
public:
	Sample();
	~Sample();
	unsigned int Repeat;
	float Time;
	std::string Pattern;
	std::string Side;
	void Deserialize(const Json::Value& root) override;
	void Serialize(const Json::Value& root) override;
};