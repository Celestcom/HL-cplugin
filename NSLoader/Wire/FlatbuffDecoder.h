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
#include "Locator.h"
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
					float effectDuration = e->duration();
					//SPECIAL CASE: DOOM BUZZ is continuous
					//And if the duration isn't set, we need to cap it at something
					//We could also do this inside the engine
					//But this is a temporary hack until firmware is fixed
					if (e->effect() == 666) {

						if (effectDuration <= 0.0f) {
							effectDuration = 0.2f;
						}
					}
					auto effect = Locator::getTranslator().ToString(e->effect());
					outEvents.push_back(
						SuitEvent(BasicHapticEvent(e->time(), e->strength(),  effectDuration, e->area(), effect))
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

