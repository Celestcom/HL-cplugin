#pragma once
#include <vector>
#include "IJsonSerializable.h"
#include <algorithm>


enum class AreaFlag  : uint32_t {
	None = 0,
	Forearm_Left = 1 << 0,
	Upper_Arm_Left = 1 << 1,
	Shoulder_Left = 1 << 2,
	Back_Left = 1 << 3,
	Chest_Left = 1 << 4,
	Upper_Ab_Left = 1 << 5,
	Mid_Ab_Left = 1 << 6,
	Lower_Ab_Left = 1 << 7,

	Forearm_Right = 1 << 16,
	Upper_Arm_Right = 1 << 17,
	Shoulder_Right = 1 << 18,
	Back_Right = 1 << 19,
	Chest_Right = 1 << 20,
	Upper_Ab_Right = 1 << 21,
	Mid_Ab_Right = 1 << 22,
	Lower_Ab_Right = 1 << 23,

	Forearm_Both = Forearm_Left | Forearm_Right,
	Upper_Arm_Both = Upper_Arm_Left | Upper_Arm_Right,
	Shoulder_Both = Shoulder_Left | Shoulder_Right,
	Back_Both = Back_Left | Back_Right,
	Chest_Both = Chest_Left | Chest_Right,
	Upper_Ab_Both = Upper_Ab_Left | Upper_Ab_Right,
	Mid_Ab_Both = Mid_Ab_Left | Mid_Ab_Right,
	Lower_Ab_Both = Lower_Ab_Left | Lower_Ab_Right,

	Left_All = 0x000000FF,
	Right_All = 0x00FF0000,
	All_Areas = Left_All | Right_All
};

inline AreaFlag& operator|=(AreaFlag& a, const AreaFlag b)
{
	a = static_cast<AreaFlag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	return a;
}
class MalformedHapticsFileException : public std::runtime_error {
public:
	MalformedHapticsFileException(const std::string& detail) : std::runtime_error(detail) {}
	MalformedHapticsFileException(const std::string& package, const std::string& detail) : std::runtime_error(std::string(package + ": " + detail)) {}

};

struct TinyEffect {
	float Time;
	float Strength;
	float Duration;
	unsigned int Area;
	std::string Effect;
	TinyEffect(float time, float strength, float duration, std::string effect, unsigned int area) :Time(time), Strength(strength), Duration(duration), Effect(effect), Area(area) {}
};
struct BasicHapticEvent {
	float Time;
	float Strength;
	float Duration;
	uint32_t Area;
	std::string Effect;
	BasicHapticEvent(float time, float strength, float duration, uint32_t area, std::string effect);
	BasicHapticEvent();
};
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


class HapticFrame 
{
public:
	HapticFrame(float time, std::vector<JsonSequenceAtom> frame, AreaFlag area, unsigned int priority, float strength);
	~HapticFrame();
	float Time;
	float OriginalTime;
	float Strength;
	AreaFlag Area;
	unsigned int Priority;
	std::vector<JsonSequenceAtom> Frame;

};
class HapticSample
{
public:
	unsigned int Priority;
	float Time;
	float OriginalTime;
	std::vector<HapticFrame> Frames;
	HapticSample(float time, std::vector<HapticFrame> frames, unsigned int priority);
	~HapticSample();
};

float GetTotalPlayTime(const std::vector<JsonSequenceAtom>& atoms);

float GetTotalPlayTime(const std::vector<HapticFrame>& frames);
float GetTotalPlayTime(const std::vector<HapticSample>& samples);
float GetTotalPlayTime(const std::vector<TinyEffect>& effects);
class JsonPatternAtom : public IJsonSerializable {
public:
	float Time;
	std::string Sequence;
	std::string Area;
	float Strength;
	void Deserialize(const Json::Value& root);
	void Serialize(const Json::Value& root) {};
};

class JsonExperienceAtom : public IJsonSerializable {
public: 
	float Time;
	std::string Pattern;
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

class JsonExperience {
public:
	JsonExperience() = default;
	JsonExperience(std::string name, std::vector<JsonExperienceAtom> atoms) : _name(name), _atoms(atoms) {};
	std::string Name() const { return _name; }
	std::vector<JsonExperienceAtom> JsonAtoms() const { return _atoms; }
private:
	std::string _name;
	std::vector<JsonExperienceAtom> _atoms;
};
namespace NullSpace {

	
	class Node {
	public:
		enum class EffectType { Effect, Sequence, Pattern, Experience };
		Node::EffectType Type;
		std::vector<Node> Children;
		float Time;
		std::string Effect;
		float Strength;
		float Duration;
		uint32_t Area;
		void Propogate(float time, float strength, uint32_t area);
		Node() {}
		Node(Node::EffectType nodeType) : Type(nodeType), _marked(false) {}
		bool Marked() {
			return _marked;
		}

		void Mark() {
			_marked = true;
		}

		void Unmark() {
			_marked = false;
		}
	private:
		bool _marked;
	};
	void Propogate(NullSpace::Node& rootNode);
	std::vector<NullSpace::Node*> Flatten(NullSpace::Node& rootNode);
	std::vector<TinyEffect> EncodeTinyEffects(std::vector<Node*>& input);
	

	
}

class PackedSequence {
public:
	PackedSequence(std::string name, std::vector<JsonSequenceAtom> atoms, AreaFlag a, float strength) :_name(name), _atoms(atoms), _area(a), _strength(strength) {};
	PackedSequence() = default;
	std::string Name() const { return _name; }
	::AreaFlag Area() const { return _area; }
	float Strength() const { return _strength; }
	std::vector<JsonSequenceAtom> JsonAtoms() const { return _atoms; }
private:
	std::string _name;
	::AreaFlag _area;
	float _strength;
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
