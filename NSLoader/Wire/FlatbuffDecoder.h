#pragma once
#include "flatbuffers\flatbuffers.h"
#include "HapticEffect_generated.h"
#include "HapticFrame_generated.h"
#include "HapticPacket_generated.h"
#include "HapticSample_generated.h"
#include "ClientStatusUpdate_generated.h"
#include "Experience_generated.h"
#include "EnginePacket_generated.h"
#include "SuitStatusUpdate_generated.h"
#include "TrackingUpdate_generated.h"
#include "IntermediateHapticFormats.h"
#include "Events_generated.h"
#include "MixedHapticFrame_generated.h"
#include "MixedPattern_generated.h"
#include "MixedSequence_generated.h"

#include "NSLoader.h"
#include <boost/variant.hpp>
class FlatbuffDecoder
{
public:

	struct EngineCommand {
		NullSpace::HapticFiles::EngineCommand Command;
		EngineCommand(short c) : Command(NullSpace::HapticFiles::EngineCommand(c)) {}
	};
	struct HandleCommand {
		unsigned int Handle;
		NullSpace::HapticFiles::Command Command;
		HandleCommand(unsigned int h, NullSpace::HapticFiles::Command c) :Handle(h), Command(c) {}
		HandleCommand(unsigned int h, short c) : Handle(h), Command(NullSpace::HapticFiles::Command(c)) {}
	};
	typedef boost::variant<BasicHapticEvent> SuitEvent;


	FlatbuffDecoder() {}
	~FlatbuffDecoder() {}

	static bool FlatbuffDecoder::Decode(const NullSpace::HapticFiles::Tracking* tracking) {
		return tracking->enable();
	}


	static HandleCommand FlatbuffDecoder::Decode(const NullSpace::HapticFiles::HandleCommand* command) {
		return HandleCommand(command->handle(), command->command());
	}


	static std::vector<SuitEvent> FlatbuffDecoder::Decode(const NullSpace::Events::SuitEventList* eventList) {

		std::vector<SuitEvent> outEvents;
		if (eventList->events() != nullptr &&eventList->events()->size() > 0) {
			for (const auto& suitEvent : *eventList->events()) {
				switch (suitEvent->event_type()) {
				case NullSpace::Events::SuitEventType::SuitEventType_BasicHapticEvent:
				{
					auto e = static_cast<const NullSpace::Events::BasicHapticEvent*>(suitEvent->event());
					outEvents.push_back(
						SuitEvent(BasicHapticEvent(e->time(), e->strength(), e->duration(), e->area(), e->effect()->str()))
					);
					break;
				}
				default:
					//add nothing
					break;
				}

			}
		}
		return outEvents;
	}

	


	static NullSpace::Communication::SuitStatus FlatbuffDecoder::Decode(const NullSpace::Communication::SuitStatusUpdate* update) {
		return update->status();
	}
	static NSVR_InteropTrackingUpdate FlatbuffDecoder::Decode(const NullSpace::Communication::TrackingUpdate* update) {
		NSVR_InteropTrackingUpdate t = {};


		auto quat = update->chest();
		t.chest.w = quat->w();
		t.chest.x = quat->x();
		t.chest.y = quat->y();
		t.chest.z = quat->z();
		return t;

	}


};

