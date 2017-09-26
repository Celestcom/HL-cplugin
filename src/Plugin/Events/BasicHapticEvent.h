#pragma once
#include <boost/variant.hpp>
#include "ParameterizedEvent.h"
#include "PlayableEvent.h"
#include "NSLoader.h"


class BasicHapticEvent : public PlayableEvent {
public:	
	BasicHapticEvent();
	using Where = boost::variant<std::vector<uint32_t>, std::vector<uint64_t>>;
	float strength() const;
	uint32_t effectFamily() const;

	/* PlayableEvent impl */
	float time() const override;
	float duration() const override;
	NSVR_EventType type() const override;
	bool parse(const ParameterizedEvent&) override;
	void serialize(NullSpaceIPC::HighLevelEvent& event) const override;
	static constexpr NSVR_EventType descriptor = NSVR_EventType::NSVR_EventType_SimpleHaptic;

private:
	float m_time;
	float m_strength;
	float m_duration;
	Where m_area;
	std::string m_parsedEffectFamily;
	uint32_t m_requestedEffectFamily;



	virtual bool isEqual(const PlayableEvent& other) const override;


};


typedef boost::variant<BasicHapticEvent> SuitEvent;
