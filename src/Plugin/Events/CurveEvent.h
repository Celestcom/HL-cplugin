#pragma once
#include <boost/variant.hpp>
#include "ParameterizedEvent.h"
#include "PlayableEvent.h"
#include "NSLoader.h"
#pragma warning(push)
#pragma warning(disable : 4267)
#include "HighLevelEvent.pb.h"
#pragma warning(pop)

class CurveEvent : public PlayableEvent {
public:
	CurveEvent();


	NSVR_EventType type() const override;
	static constexpr NSVR_EventType descriptor = NSVR_EventType::NSVR_EventType_CurveHapticEvent;
	virtual float duration() const override;
	virtual float time() const override;
	virtual std::vector<uint32_t> area() const override;


	


	virtual bool parse(const ParameterizedEvent&) override;


	virtual void serialize(NullSpaceIPC::HighLevelEvent& event) const override;

private:
	float m_time;
	uint32_t m_area;
	float m_duration;
	std::vector<float> m_volumes;
	std::vector<float> m_timePoints;
	



	virtual bool isEqual(const PlayableEvent& other) const override;

};


