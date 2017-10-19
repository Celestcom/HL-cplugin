#include "stdafx.h"
#include "validators.h"

Validator make_xor_constraint(Validator lhs, Validator rhs) {
	return [lhs, rhs](const ParameterizedEvent& event, std::vector<HLVR_Event_KeyParseResult>* results) {
		return lhs(event, results) ^ rhs(event, results);
	};
}

Validator make_or_constraint(Validator lhs, Validator rhs) {
	return [lhs, rhs](const ParameterizedEvent& event, std::vector<HLVR_Event_KeyParseResult>* results) {
		return lhs(event, results) || rhs(event, results);
	};
}

Validator make_and_constraint(Validator lhs, Validator rhs) {
	return [lhs, rhs](const ParameterizedEvent& event, std::vector<HLVR_Event_KeyParseResult>* results) {
		return lhs(event, results) && rhs(event, results);
	};
}