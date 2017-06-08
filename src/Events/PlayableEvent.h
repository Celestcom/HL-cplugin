#pragma once

#include <stdint.h>
class PlayableEvent {
public:
	PlayableEvent() {};
	virtual ~PlayableEvent() {};
	virtual float time() const = 0;
	virtual uint32_t area() const = 0;
	virtual float duration() const = 0;
	bool operator<(const PlayableEvent& rhs) const;

};
