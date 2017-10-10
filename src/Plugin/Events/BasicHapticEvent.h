#pragma once
#include <boost/variant.hpp>
#include "ParameterizedEvent.h"
#include "PlayableEvent.h"
#include "HLVR.h"


class BasicHapticEvent : public PlayableEvent {
public:	
	BasicHapticEvent(float time);
	
	template<typename T>
	struct Loc {
		uint32_t value;
		explicit Loc(uint32_t val) : value(val) {}
		Loc() : value(0) {}
	
	};

	
	struct node {};
	struct region {};

	using Where = boost::variant<std::vector<Loc<node>>, std::vector<Loc<region>>>;
	float strength() const;
	uint32_t effectFamily() const;

	/* PlayableEvent impl */
	float duration() const override;
	HLVR_EventType type() const override;
	bool parse(const ParameterizedEvent&) override;
	std::vector<Validator> make_validators() const override;
	void serialize(NullSpaceIPC::HighLevelEvent& event) const override;
	static constexpr HLVR_EventType descriptor = HLVR_EventType::HLVR_EventType_SimpleHaptic;

private:
	float m_strength;
	float m_duration;
	Where m_area;
	std::string m_parsedEffectFamily;
	uint32_t m_requestedEffectFamily;



	virtual bool isEqual(const PlayableEvent& other) const override;


};
template<typename T>
bool operator==(const BasicHapticEvent::Loc<T>& lhs, const BasicHapticEvent::Loc<T>& rhs) { return lhs.value == rhs.value; }

typedef boost::variant<BasicHapticEvent> SuitEvent;
