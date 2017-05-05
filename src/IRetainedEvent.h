#pragma once

class IRetainedEvent {
public:
	virtual void Begin() = 0;
	virtual void Pause() = 0;
	virtual void Resume() = 0;
	virtual void Update(float dt) = 0;
	virtual bool Finished() = 0;
	virtual ~IRetainedEvent() {}
};