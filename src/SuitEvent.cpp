#include "stdafx.h"
#include "SuitEvent.h"
#include "Locator.h"
BasicHapticEvent::BasicHapticEvent(float time, float strength, float duration, uint32_t area, std::string effect) : ParameterizedEvent(),
Time(time),
Strength(strength),
Duration(duration),
Area(area),
ParsedEffectFamily(effect),
RequestedEffectFamily(Locator::getTranslator().ToEffectFamily(ParsedEffectFamily))

{}

BasicHapticEvent::BasicHapticEvent() : ParameterizedEvent(), Time(0), Strength(1), Duration(0), Area(0)
{
	ParsedEffectFamily = "click";
	RequestedEffectFamily = Locator::getTranslator().ToEffectFamily(ParsedEffectFamily);

}


bool BasicHapticEvent::doSetFloat(const char * key, float value)
{
	if (strcmp("strength", key) == 0) {
		Strength = value;
		return true;
	}
	else if (strcmp("time", key) == 0) {
		Time = value;
		return true;
	}
	else if (strcmp("duration", key) == 0) {
		Duration = value;
		return true;
	}
	else {
		return false;
	}
}

bool BasicHapticEvent::doSetInt(const char * key, int value)
{
	if (strcmp("area", key) == 0) {
		Area = value;
		return true;
	}
	else if (strcmp("effect", key) == 0) {
		std::string effect = Locator::getTranslator().ToEffectFamilyString(value);
		this->ParsedEffectFamily = effect;
		this->RequestedEffectFamily = value;
		return true;
	}

	return false;
}
