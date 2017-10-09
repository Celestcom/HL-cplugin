#pragma once
#include <boost/optional.hpp>
#include <stdint.h>
#include <array>
#include "HLVR_Forwards.h"
class ParameterizedEvent;

namespace NullSpaceIPC {
	class HighLevelEvent;
}


class PlayableEvent {
public:
	PlayableEvent() {};
	virtual ~PlayableEvent() {};

	virtual float time() const = 0;
	virtual float duration() const = 0;
	virtual HLVR_EventType type() const = 0;
	virtual bool parse(const ParameterizedEvent&) = 0;
	virtual void serialize(NullSpaceIPC::HighLevelEvent& event) const = 0;
	bool operator<(const PlayableEvent& rhs) const;

	static std::unique_ptr<PlayableEvent> make(HLVR_EventType type);

	bool operator==(const PlayableEvent& other) const;


private:
	virtual bool isEqual(const PlayableEvent& other) const = 0;
};


std::vector<uint32_t> extractRegions(const PlayableEvent& event);

bool cmp_by_duplicate(const std::unique_ptr<PlayableEvent>& lhs, const std::unique_ptr<PlayableEvent>& rhs);
bool cmp_by_time(const std::unique_ptr<PlayableEvent>& lhs, const std::unique_ptr<PlayableEvent>& rhs);

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