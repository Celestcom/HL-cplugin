#pragma once
#include <boost/optional.hpp>
#include <stdint.h>
#include <array>
#include "HLVR.h"
class ParameterizedEvent;

namespace NullSpaceIPC {
	class HighLevelEvent;
}

using Validator = std::function<bool(const ParameterizedEvent&, std::vector<HLVR_Event_KeyParseResult>*)>;

template<typename T>
using Predicate = std::function<bool(const T&)>;

template<typename T>
using Constraint = std::function<Validator(HLVR_EventKey, Predicate<T> c)>;



struct key_req_t {};
static const key_req_t key_required;


template<typename T, typename Constraint>
boost::optional<HLVR_Event_KeyParseError> validate_helper(HLVR_EventKey key, const ParameterizedEvent& event, Constraint&& constraint) {
	T value;
	if (event.TryGet(key, &value)) {
		if (!constraint(value)) {
			return HLVR_Event_KeyParseError_InvalidValue;
		}
	}
	else {
		return HLVR_Event_KeyParseError_WrongValueType;
	}


	return boost::none;
}



template<typename T, typename Constraint>
boost::optional<HLVR_Event_KeyParseError> validate(HLVR_EventKey key, const ParameterizedEvent& event, Constraint&& constraint, key_req_t req) {

	if (!event.HasKey(key)) {
		return HLVR_Event_KeyParseError_KeyRequired;
	}

	return validate_helper<T>(key, event, std::move(constraint));
}

template<typename T, typename Constraint>
boost::optional<HLVR_Event_KeyParseError> validate(HLVR_EventKey key, const ParameterizedEvent& event, Constraint&& constraint) {

	if (!event.HasKey(key)) {
		return boost::none;
	}
	return validate_helper<T>(key, event, std::move(constraint));
}


template<typename T>
Validator make_constraint(HLVR_EventKey key, Predicate<T> c) {
	return[key, fn = std::move(c)](const ParameterizedEvent& event, std::vector<HLVR_Event_KeyParseResult>* results) {
		if (auto error = validate<T>(key, event, std::move(fn))) {
			results->push_back(HLVR_Event_KeyParseResult{ key, *error });
			return false;
		}
		return true;
	};
}

template<typename T>
Validator make_required_constraint(HLVR_EventKey key, Predicate<T> c) {
	return[key, fn = std::move(c)](const ParameterizedEvent& event, std::vector<HLVR_Event_KeyParseResult>* results) {
		if (auto error = validate<T>(key, event, std::move(fn), key_required)) {
			results->push_back(HLVR_Event_KeyParseResult{ key, *error });
			return false;
		}
		return true;
	};
}


Validator make_xor_constraint(Validator lhs, Validator rhs);
Validator make_or_constraint(Validator lhs, Validator rhs);

Validator make_and_constraint(Validator lhs, Validator rhs);

struct TargetRegions {
	std::vector<uint32_t> regions;
};

inline bool operator==(const TargetRegions& lhs, const TargetRegions& rhs) {
	return lhs.regions == rhs.regions;
}

struct TargetNodes {
	std::vector<uint32_t> nodes;
};

inline bool operator==(const TargetNodes& lhs, const TargetNodes& rhs) {
	return lhs.nodes == rhs.nodes;
}




using Target = boost::variant<TargetRegions, TargetNodes>;

class PlayableEvent {
public:
	PlayableEvent(float time);
	virtual ~PlayableEvent() = default;

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

	//Return total duration of the event in fractional seconds. Can be an estimate. 
	virtual float duration() const = 0;


	static std::unique_ptr<PlayableEvent> make(HLVR_EventType type, float timeOffset);



private:
	float m_time;
	Target m_target;
	virtual std::vector<Validator> makeValidators() const = 0;
	virtual void doSerialize(NullSpaceIPC::HighLevelEvent& event) const = 0;
	virtual bool doParse(const ParameterizedEvent&) = 0;
	virtual bool isEqual(const PlayableEvent& other) const = 0;
};



bool cmp_by_duplicate(const std::unique_ptr<PlayableEvent>& lhs, const std::unique_ptr<PlayableEvent>& rhs);
bool cmp_by_time(const std::unique_ptr<PlayableEvent>& lhs, const std::unique_ptr<PlayableEvent>& rhs);

//todo: actually use this factory
class PlayableEventFactory {
public:
	template<typename Derived>
	void registerType(HLVR_EventType type) {
		static_assert(std::is_base_of<PlayableEvent, Derived>::value, "Derived must be derived from PlayableEvent!");
		_createFuncs[name] = []() {
			return std::make_unique<Derived>();
		};
	}
	std::unique_ptr<PlayableEvent> create(HLVR_EventType type) {
		auto it = _createFuncs.find(type);
		if (it != _createFuncs.end()) {
			return (it->second)();
		}
		return std::unique_ptr<PlayableEvent>{};
	}
private:
	using CreateFunc = std::function<std::unique_ptr<PlayableEvent>(void)>;
	std::unordered_map<HLVR_EventType, CreateFunc> _createFuncs;
};