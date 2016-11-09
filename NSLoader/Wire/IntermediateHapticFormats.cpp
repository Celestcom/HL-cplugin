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
