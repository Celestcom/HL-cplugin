#include "stdafx.h"
#include "CurveEvent.h"
#include "AreaFlags.h"

CurveEvent::CurveEvent() : 
	PlayableEvent(),
	m_area(0),
	m_duration(0),
	m_time(0),
	m_timePoints(),
	m_volumes()
{
	
}





float CurveEvent::time() const
{
	return m_time;
}

uint32_t CurveEvent::area() const
{
	return m_area;
}

bool CurveEvent::parse(const ParameterizedEvent& ev)
{
	m_time = ev.Get("time", 0.0f);
	m_timePoints = ev.Get("time-offsets", std::vector<float>({}));
	m_volumes = ev.Get("magnitudes", std::vector<float>({}));
	m_area = ev.Get("area", (int)AreaFlag::None);
	return true;
}

void CurveEvent::serialize(NullSpaceIPC::HighLevelEvent& event) const
{
	using namespace NullSpaceIPC;
	auto curve = event.mutable_curve_haptic();
	
	auto regions = extractRegions(*this);
	for (const auto& region : regions) {
		curve->add_regions(region);
	}

	for (auto i = 0; i < m_timePoints.size(); i++) {
		auto sample = curve->add_samples();
		sample->set_time(m_timePoints[i]);
		sample->set_magnitude(m_volumes[i]);
	}
}

bool CurveEvent::isEqual(const PlayableEvent& other) const
{
	const auto& ev = static_cast<const CurveEvent&>(other);
	return
		m_time == ev.m_time
		&& m_area == ev.m_area
		&& m_duration == ev.m_duration
		&& m_volumes == ev.m_volumes
		&& m_timePoints == ev.m_timePoints;
}

NSVR_EventType CurveEvent::type() const
{
	return descriptor;
}

float CurveEvent::duration() const
{
	return m_duration;
}
