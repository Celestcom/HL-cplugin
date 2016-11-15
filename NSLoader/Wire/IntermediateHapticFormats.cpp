#include "IntermediateHapticFormats.h"
#include "json.h"


void JsonSequenceAtom::Serialize(const Json::Value& root) {

}

void JsonSequenceAtom::Deserialize(const Json::Value& root) {
	this->Time = root.get("time", 0.0f).asFloat();
	this->Effect = root.get("effect", "INVALID_EFFECT").asString();
	this->Strength = root.get("strength", 1.0).asFloat();
	this->Duration = root.get("duration", 0.0f).asFloat();
	this->Repeat = root.get("repeat", 1).asInt();

}




void JsonPatternAtom::Deserialize(const Json::Value & root)
{
	this->Time = root.get("time", 0.0).asFloat();
	this->Sequence = root.get("sequence", "UNKNOWN_SEQUENCE").asString();
	this->Area = root.get("area", "UNKNOWN_AREA").asString();

}

void JsonExperienceAtom::Deserialize(const Json::Value& root) {
	this->Time = root.get("time", 0.0).asFloat();
	this->Pattern = root.get("pattern", "UNKNOWN_PATTERN").asString();

}
HapticFrame::HapticFrame(float time, std::vector<JsonSequenceAtom> frame, AreaFlag a, unsigned int priority) : Time(time), OriginalTime(time), Priority(priority), Frame(frame), Area(a)
{
}

HapticFrame::~HapticFrame()
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

float GetTotalPlayTime(const std::vector<JsonSequenceAtom>& atoms) {
	float endOfLastEffect = 0.0;
	for (const auto& item : atoms) {
		//fudge factor of quarter of second for oneshots
		float endTime = (item.Duration + 0.25) + item.Time;
		endOfLastEffect = std::max(endOfLastEffect, endTime);
	}
	return endOfLastEffect;
}