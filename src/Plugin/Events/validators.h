#pragma once

#include <vector>
#include <functional>
#include <boost/optional.hpp>
#include "ParameterizedEvent.h"
#include "HLVR.h"


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
Validator make_optional_constraint(HLVR_EventKey key, Predicate<T> c) {
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

