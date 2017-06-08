#include "stdafx.h"
#include "PlayableEvent.h"

bool PlayableEvent::operator<(const PlayableEvent & rhs) const
{
	return this->time() < rhs.time();
}
