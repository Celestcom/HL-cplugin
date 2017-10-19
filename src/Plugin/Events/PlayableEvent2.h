#pragma once
#include "HighLevelEvent.pb.h"

class PlayableEvent2 {
public:
	//virtual void serialize(NullSpaceIPC::HighLevelEvent& event) = 0;
	bool validate() {}
	virtual ~PlayableEvent2() = default;
private:
//	virtual void makeValidators() const = 0;

};