#pragma once
#include <boost/optional.hpp>
#include <stdint.h>
#include <array>
#include "HLVR.h"
#include "validators.h"
#include "target.h"

class ParameterizedEvent;

namespace NullSpaceIPC {
	class HighLevelEvent;
}



class PlayableEvent {
public:
	PlayableEvent(float time);
	virtual ~PlayableEvent() = default;
	
	//Return total duration of the event in fractional seconds. Can be an estimate. 
	virtual float duration() const = 0;
	
	
	//Return time offset of the event in fractional seconds
	float time() const;

	//Perform a parse of the given ParameterizedEvent, but don't actually create a real event - just throw results in 'result'
	void debug_parse(const ParameterizedEvent& event, HLVR_Event_ValidationResult* result) const;

	//Perform a true parse of the ParameterizedEvent
	bool parse(const ParameterizedEvent& e);

	//Serialize the event into our transport protocol message
	void serialize(NullSpaceIPC::HighLevelEvent& event) const;
	
	//Compare events based on time offset
	bool operator<(const PlayableEvent& rhs) const;

	//Compare event for equality (same time, same type, and derived == other derived)
	bool operator==(const PlayableEvent& other) const;

	

	//Our super-simple basic factory that allows you to forget to add types, etc.
	static std::unique_ptr<PlayableEvent> make(HLVR_EventType type, float timeOffset);



private:
	float m_time;
	Target m_target;
	
	virtual std::vector<Validator> makeValidators() const { return std::vector<Validator>{}; }
	virtual void doSerialize(NullSpaceIPC::HighLevelEvent& event) const = 0;
	virtual bool doParse(const ParameterizedEvent&) = 0;
	virtual bool isEqual(const PlayableEvent& other) const = 0;
};

 
//todo: actually use this factory..? Or not.
//class PlayableEventFactory {
//public:
//	template<typename Derived>
//	void registerType(HLVR_EventType type) {
//		static_assert(std::is_base_of<PlayableEvent, Derived>::value, "Derived must be derived from PlayableEvent!");
//		_createFuncs[name] = []() {
//			return std::make_unique<Derived>();
//		};
//	}
//	std::unique_ptr<PlayableEvent> create(HLVR_EventType type) {
//		auto it = _createFuncs.find(type);
//		if (it != _createFuncs.end()) {
//			return (it->second)();
//		}
//		return std::unique_ptr<PlayableEvent>{};
//	}
//private:
//	using CreateFunc = std::function<std::unique_ptr<PlayableEvent>(void)>;
//	std::unordered_map<HLVR_EventType, CreateFunc> _createFuncs;
//};