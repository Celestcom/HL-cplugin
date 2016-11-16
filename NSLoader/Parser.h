#pragma once
#include <vector>
#include <unordered_map>

#include "HapticDirectoryTools.h"
#include "Enums.h"
#include "Wire\IntermediateHapticFormats.h"
using namespace HapticDirectoryTools;




enum class HapticFileType
{
	Pattern = 0,
	Sequence = 1,
	Experience = 2
};


class PackageNotFoundException : public std::runtime_error {
public:
	PackageNotFoundException(const std::string& package) : std::runtime_error(std::string("The package " + package + " was not found!").c_str()) {}
	
};
class FileNotFoundException : public std::runtime_error {
public:
	FileNotFoundException(const std::string& notFoundHaptic, const std::string& notFoundHapticPackage) : std::runtime_error(std::string("could not find " + notFoundHaptic + " in package " + notFoundHapticPackage).c_str()) {}

};
class MalformedHapticsFileException : public std::runtime_error {
public:
	MalformedHapticsFileException(const std::string& detail) : std::runtime_error(detail) {}
	MalformedHapticsFileException(const std::string& package, const std::string& detail) : std::runtime_error(std::string(package +": " + detail)) {}

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
	//todo: implement experiences
	std::vector<JsonExperienceAtom> ParseExperience(boost::filesystem::path);
	

private:
	std::string _basePath;
	std::unordered_map<std::string, boost::filesystem::path> _paths;

};