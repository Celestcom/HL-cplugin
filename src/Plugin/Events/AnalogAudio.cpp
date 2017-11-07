#include "stdafx.h"
#include "AnalogAudio.h"
#pragma warning(push)
#pragma warning(disable : 4267)
#include "HighLevelEvent.pb.h"
#pragma warning(pop)


BeginAnalogAudio::BeginAnalogAudio(float time) : PlayableEvent(time)
{
}

void BeginAnalogAudio::doSerialize(NullSpaceIPC::HighLevelEvent & event) const
{
	auto locational = event.mutable_locational_event();
	auto beginAnalog = locational->mutable_begin_analog_audio();
}

bool BeginAnalogAudio::doParse(const ParameterizedEvent & event)
{
	return true;
}

EndAnalogAudio::EndAnalogAudio(float time) : PlayableEvent(time)
{
}

void EndAnalogAudio::doSerialize(NullSpaceIPC::HighLevelEvent & event) const
{
	auto locational = event.mutable_locational_event();
	auto end = locational->mutable_end_analog_audio();
}

bool EndAnalogAudio::doParse(const ParameterizedEvent &)
{
	return true;
}
