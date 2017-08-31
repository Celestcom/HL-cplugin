#pragma once
#include "SharedTypes.h"
#include <mutex>
class Encoder {
public: 



	static NullSpace::SharedMemory::ExecutionCommand Decode(const NullSpace::HapticFiles::ExecutionCommand* effect) {
		NullSpace::SharedMemory::ExecutionCommand c;
		c.Command = effect->command();
		c.Effect = effect->effect();
		c.Location = effect->location();
		return c;
		
	}

private:

};