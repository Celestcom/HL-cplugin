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



	FlatbuffDecoder() {}
	~FlatbuffDecoder() {}

	static bool FlatbuffDecoder::Decode(const NullSpace::HapticFiles::Tracking* tracking) {
		return tracking->enable();
	}



	


	static NullSpace::Communication::SuitStatus FlatbuffDecoder::Decode(const NullSpace::Communication::SuitStatusUpdate* update) {
		return update->status();
	}
	static NSVR_TrackingUpdate FlatbuffDecoder::Decode(const NullSpace::Communication::TrackingUpdate* update) {
		NSVR_TrackingUpdate t = {};


		for (const auto& q : *update->quaternions()) {
			NullSpaceDLL::Quaternion* quat = nullptr;

			switch (q->id()) {
			case NullSpace::Communication::ImuId_Chest:
				quat = &t.chest;
				break;
			case NullSpace::Communication::ImuId_Left_Forearm:
				quat = &t.left_forearm;
				break;
			case NullSpace::Communication::ImuId_Left_Upper_Arm:
				quat = &t.left_upper_arm;
				break;
			case NullSpace::Communication::ImuId_Right_Forearm:
				quat = &t.right_forearm;
				break;
			case NullSpace::Communication::ImuId_Right_Upper_Arm:
				quat = &t.right_upper_arm;
				break;
			default:
				break;
			}

			if (quat != nullptr) {
				quat->w = q->w();
				quat->x = q->x();
				quat->y = q->y();
				quat->z = q->z();
			}
		}
	
		return t;

	}


};

