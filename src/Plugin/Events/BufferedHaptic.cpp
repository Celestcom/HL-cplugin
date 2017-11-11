#include "stdafx.h"
#include "BufferedHaptic.h"
#pragma warning(push)
#pragma warning(disable : 4267)
#include "HighLevelEvent.pb.h"
#pragma warning(pop)
BufferedHaptic::BufferedHaptic(float time) : PlayableEvent(time), m_samples(), m_frequency(1.0)
{
}

float BufferedHaptic::duration() const
{
	return m_samples.size() / m_frequency;
}

std::vector<Validator> BufferedHaptic::makeValidators() const
{
	return{
		make_optional_constraint<float>(HLVR_EventKey_BufferedHaptic_Frequency_Float, [](float f) { return f > 0.0f; }),
		make_optional_constraint<std::vector<float>>(HLVR_EventKey_BufferedHaptic_Samples_Floats, [](const auto& samples) { return samples.size() > 0; })
	};
}

void BufferedHaptic::doSerialize(NullSpaceIPC::HighLevelEvent& event) const
{
	auto loc = event.mutable_locational_event();
	auto buf = loc->mutable_buffered_haptic();
	buf->set_frequency(m_frequency);
	
	auto samples = buf->mutable_samples();
	
	for (const float s : m_samples) {
		samples->Add(s);
	}
}

void BufferedHaptic::doParse(const ParameterizedEvent & ev)
{
	m_frequency = ev.GetOr(HLVR_EventKey_BufferedHaptic_Frequency_Float, 60.0f);
	m_samples = ev.GetOr(HLVR_EventKey_BufferedHaptic_Samples_Floats, std::vector<float>{});
}

bool BufferedHaptic::isEqual(const PlayableEvent & other) const
{
	const auto& ev = static_cast<const BufferedHaptic&>(other);
	return m_frequency == ev.m_frequency && m_samples == ev.m_samples;
}
