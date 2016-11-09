#pragma once
#include <vector>
#include "IJsonSerializable.h"
typedef unsigned int Area;

template<typename T>
struct TimeIndex {
	float Time;
	T Haptic;
	TimeIndex(float time, T haptic) :Time(time), Haptic(haptic) {};
};
class JsonSequenceAtom : public IJsonSerializable {
public:
	float Time;
	std::string Effect;
	float Strength;
	float Duration;
	int Repeat;
	JsonSequenceAtom() = default;
	JsonSequenceAtom(float t, std::string e, float s, float d, int r) :Time(t), Effect(e), Strength(s), Duration(d), Repeat(r) {};
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



class JsonPatternAtom : public IJsonSerializable {
public:
	float Time;
	std::string Sequence;
	std::string Area;
	void Deserialize(const Json::Value& root);
	void Serialize(const Json::Value& root) {};
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


class PackedExperience {
public:
	PackedExperience(std::string name, std::vector<TimeIndex<PackedPattern>> atoms) :_name(name), _atoms(atoms) {};
	PackedExperience() = default;
	std::string Name() const { return _name; }
	std::vector<TimeIndex<PackedPattern>> JsonAtoms() const { return _atoms; }
private:
	std::string _name;
	std::vector<TimeIndex<PackedPattern>> _atoms;
};