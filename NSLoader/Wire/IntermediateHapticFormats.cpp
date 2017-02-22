#include "IntermediateHapticFormats.h"
#include "json.h"
#include <functional>


//todo: These want to have an actual schema system. Could get another dependency Valijson
void jsonGet(float& t, const Json::Value& root, std::string key, float defaultVal, std::string malformErrorMessage)
{
	if (root.isMember(key)) {
		t = root.get(key, defaultVal).asFloat();
	}
	else {
		throw MalformedHapticsFileException(malformErrorMessage);
	}
}



void jsonGet(std::string& t, const Json::Value& root, std::string key, std::string defaultVal, std::string malformErrorMessage)
{
	if (root.isMember(key)) {
		t = root.get(key, defaultVal).asString();
	}
	else {
		throw MalformedHapticsFileException(malformErrorMessage);
	}
}


void jsonGet(int& t, const Json::Value& root, std::string key, int defaultVal, std::string malformErrorMessage)
{
	if (root.isMember(key)) {
		t = root.get(key, defaultVal).asInt();
	}
	else {
		throw MalformedHapticsFileException(malformErrorMessage);
	}
}


void JsonSequenceAtom::Serialize(const Json::Value& root) {

}

void JsonSequenceAtom::Deserialize(const Json::Value& root) {
	jsonGet(this->Time, root, "time", 0.0, "key 'time' must be present in this sequence definition");
	jsonGet(this->Effect, root, "effect", "INVALID_EFFECT", "key 'effect' must be present in this sequence definition");
	this->Strength = root.get("strength", 1.0).asFloat();
	this->Duration = root.get("duration", 0.0f).asFloat();
	this->Repeat = root.get("repeat", 1).asInt();

}


void JsonPatternAtom::Deserialize(const Json::Value & root)
{
	jsonGet(this->Time, root, "time", 0.0, "key 'time' must be present in this sequence-item");
	jsonGet(this->Sequence, root, "sequence", "INVALID_SEQUENCE", "key 'sequence' must be present in this sequence-item");
	jsonGet(this->Area, root, "area", "INVALID_AREA", "key 'area' must be present in this sequence-item");
	this->Strength = root.get("strength", 1.0).asFloat();
}

void JsonExperienceAtom::Deserialize(const Json::Value& root) {
	jsonGet(this->Time, root, "time", 0.0, "key 'time' must be present in this pattern-item");
	jsonGet(this->Pattern, root, "pattern", "INVALID_PATTERN", "key 'pattern' must be present in this pattern-item");

}
HapticFrame::HapticFrame(float time, std::vector<JsonSequenceAtom> frame, AreaFlag a, unsigned int priority, float strength) : Time(time), OriginalTime(time), Priority(priority), Frame(frame), Area(a), Strength(strength)
{
}

HapticFrame::~HapticFrame()
{
}


HapticSample::HapticSample(float time, std::vector<HapticFrame> frames, unsigned int priority) :
	Priority(priority), Time(time), OriginalTime(time), Frames(frames)

{
}

HapticSample::~HapticSample()
{
}



float GetTotalPlayTime(const std::vector<HapticFrame>& frames) {
	float endOfLastEffect = 0.0;
	for (const auto& item : frames) {
		float endTime = GetTotalPlayTime(item.Frame) + item.Time;
		endOfLastEffect = std::max(endOfLastEffect, endTime);
	}
	return endOfLastEffect;
}

float GetTotalPlayTime(const std::vector<TinyEffect>& effects) {
	float endOfLastEffect = 0.0;
	for (const auto& item : effects) {
		//fudge factor of quarter of second for oneshots
		float endTime = (item.Duration + 0.25f) + item.Time;
		endOfLastEffect = std::max(endOfLastEffect, endTime);
	}
	return endOfLastEffect;
}



float GetTotalPlayTime(const std::vector<JsonSequenceAtom>& atoms) {
	float endOfLastEffect = 0.0;
	for (const auto& item : atoms) {
		//fudge factor of quarter of second for oneshots
		float endTime = (item.Duration + 0.25f) + item.Time;
		endOfLastEffect = std::max(endOfLastEffect, endTime);
	}
	return endOfLastEffect;
}




float GetTotalPlayTime(const std::vector<HapticSample>& atoms) {
	float endOfLastEffect = 0.0;
	for (const auto& item : atoms) {
		//fudge factor of quarter of second for oneshots
		float endTime = (GetTotalPlayTime(item.Frames) + 0.25f) + item.Time;
		endOfLastEffect = std::max(endOfLastEffect, endTime);
	}
	return endOfLastEffect;
}


void NullSpace::Node::Propogate(float time, float strength, uint32_t area)
{
	Time += time;
	Strength *= strength;
	if (this->Type == Node::EffectType::Effect) {
		Area = area;
	}
	for (auto& child : Children) {
		child.Propogate(Time, Strength, Area);
	}
}




void NullSpace::Propogate(NullSpace::Node & rootNode)
{
	rootNode.Propogate(0.0f, 1.0f, uint32_t(AreaFlag::None));

}

std::vector<NullSpace::Node*> NullSpace::Flatten(NullSpace::Node& rootNode)
{
	//In order to flatten the hierarchy, we must add all the leaf nodes to a list
	//We will not have any cycles 
	//Take a pointer to the root node, and hold on to the list result
	typedef std::function<void(Node&, std::vector<Node*>&)> DFSFunc;

	DFSFunc visitor = [&](Node& node, std::vector<Node*>& result) {
		if (node.Children.empty()) {
			result.push_back(&node);
			return;
		}
		for (auto& child : node.Children) {
			visitor(child, result);
		}
	};

	std::vector<Node*> result;
	visitor(rootNode, result);

	//Sort the flattened effects by their time offsets
	std::sort(result.begin(), result.end(), [](const Node* lhs, const Node* rhs) {return lhs->Time < rhs->Time; });
	return result;
}

std::vector<TinyEffect> NullSpace::EncodeTinyEffects(std::vector<Node*>& input)
{
	std::vector<TinyEffect> effects;
	effects.reserve(input.size());
	for (const auto& e : input) {
		effects.push_back(TinyEffect(e->Time, e->Strength, e->Duration, e->Effect, e->Area));
	}
	return effects;
}

BasicHapticEvent::BasicHapticEvent(float time, float strength, float duration, uint32_t area, std::string effect) :
	Time(time),
	Strength(strength),
	Duration(duration),
	Area(area),
	Effect(effect)

{}