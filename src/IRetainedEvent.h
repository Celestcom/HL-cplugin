#pragma once
#include "EffectCommand.pb.h"

class IRetainedEvent {
public:
	virtual void Begin() = 0;
	virtual void Pause() = 0;
	virtual void Resume() = 0;
	virtual std::vector<NullSpaceIPC::EffectCommand> Update(float dt) = 0;
	virtual bool Finished() = 0;
	virtual ~IRetainedEvent() {}
};