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

void BeginAnalogAudio::doParse(const ParameterizedEvent & event)
{
}

EndAnalogAudio::EndAnalogAudio(float time) : PlayableEvent(time)
{
}

void EndAnalogAudio::doSerialize(NullSpaceIPC::HighLevelEvent & event) const
{
	auto locational = event.mutable_locational_event();
	auto end = locational->mutable_end_analog_audio();
}

void EndAnalogAudio::doParse(const ParameterizedEvent &)
{
}
