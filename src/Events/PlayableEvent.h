#pragma once
#include <boost/optional.hpp>
#include <stdint.h>
#include <array>
#include "NSLoader_fwds.h"
class ParameterizedEvent;

namespace NullSpaceIPC {
	class HighLevelEvent;
}
class PlayableEvent {
public:
	PlayableEvent() {};
	virtual ~PlayableEvent() {};

	virtual float time() const = 0;
	virtual uint32_t area() const = 0;
	virtual float duration() const = 0;
	virtual NSVR_EventType type() const = 0;
	virtual bool parse(const ParameterizedEvent&) = 0;
	virtual void serialize(NullSpaceIPC::HighLevelEvent& event) const = 0;
	bool operator<(const PlayableEvent& rhs) const;

	static std::unique_ptr<PlayableEvent> make(NSVR_EventType type);

	bool operator==(const PlayableEvent& other) const;


private:
	virtual bool isEqual(const PlayableEvent& other) const = 0;
};


std::vector<std::string> extractRegions(const PlayableEvent& event);

bool cmp_by_duplicate(const std::unique_ptr<PlayableEvent>& lhs, const std::unique_ptr<PlayableEvent>& rhs);
bool cmp_by_time(const std::unique_ptr<PlayableEvent>& lhs, const std::unique_ptr<PlayableEvent>& rhs);

