#pragma once
#include "EffectCommand.pb.h"

class IRetainedEvent {
public:
	virtual void Begin(CommandBuffer* buffer) = 0;
	virtual void Pause(CommandBuffer* buffer) = 0;
	virtual void Resume(CommandBuffer* buffer) = 0;
	virtual void Update(float dt) = 0;
	virtual bool Finished() const = 0;
	virtual ~IRetainedEvent() {}
};