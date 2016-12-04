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

#include "MixedHapticFrame_generated.h"
#include "MixedPattern_generated.h"
#include "MixedSequence_generated.h"

#include "EncodingOperations.h"
class FlatbuffDecoder
{
public:
	FlatbuffDecoder() {}
	~FlatbuffDecoder() {}

	static bool FlatbuffDecoder::Decode(const NullSpace::HapticFiles::Tracking* tracking) {
		return tracking->enable();
	}
	static JsonSequenceAtom FlatbuffDecoder::Decode(const NullSpace::HapticFiles::HapticEffect* effect) {
		return JsonSequenceAtom(effect->time(), effect->effect()->str(), effect->strength(), effect->duration(), effect->repeat());
	}
	static std::vector<JsonSequenceAtom> FlatbuffDecoder::Decode(const NullSpace::HapticFiles::Sequence* sequence) {
		std::vector<JsonSequenceAtom> atoms;
		atoms.reserve(sequence->items()->size());
		auto seqs = sequence->items();
		for (const auto& e : *seqs) {
			atoms.push_back(Decode(e));
		}
		return atoms;
	}

	static std::vector<JsonSequenceAtom> FlatbuffDecoder::Decode(const NullSpace::HapticFiles::Mixed::Sequence* sequence, std::unique_ptr<DependencyResolver>& resolver) {
		std::vector<JsonSequenceAtom> atoms;
		if (sequence->effect_type() == NullSpace::HapticFiles::Mixed::EffectValuesOrNameReference_ListOfHapticEffects) {
			auto efs = static_cast<const NullSpace::HapticFiles::Mixed::ListOfHapticEffects*>(sequence->effect());
			for (const auto e : *efs->items()) {
				atoms.push_back(Decode(e));
			}
			return atoms;
		}
		else {
			auto ef = static_cast<const NullSpace::HapticFiles::Mixed::SeqRef*>(sequence->effect());
			std::string effect_id = ef->symbol()->str();
			return resolver->FetchCachedSequence(effect_id);
		}

	}


	static NullSpaceDLL::HandleCommand FlatbuffDecoder::Decode(const NullSpace::HapticFiles::HandleCommand* command) {
		return NullSpaceDLL::HandleCommand(command->handle(), command->command());
	}



	static std::vector<HapticFrame> FlatbuffDecoder::Decode(const NullSpace::HapticFiles::Pattern* pattern)
	{
		std::vector<HapticFrame> frames;
		const auto items = pattern->items();
		frames.reserve(items->size());

		for (const auto& frame : *items) {

			auto seq = Decode(frame->sequence());

			frames.push_back(HapticFrame(frame->time(), seq, AreaFlag(frame->area()), 1));
		}

		return frames;
	}



	static std::vector<HapticSample> FlatbuffDecoder::Decode(const NullSpace::HapticFiles::Experience* experience) {
		std::vector<HapticSample> samples;
		auto items = experience->items();
		samples.reserve(items->size());

		for (const auto& sample : *items) {
			samples.push_back(HapticSample(sample->time(), Decode(sample->pattern()), 1));

		}
		return samples;
	}


	static NullSpace::Communication::SuitStatus FlatbuffDecoder::Decode(const NullSpace::Communication::SuitStatusUpdate* update) {
		return update->status();
	}
	static NullSpaceDLL::InteropTrackingUpdate FlatbuffDecoder::Decode(const NullSpace::Communication::TrackingUpdate* update) {
		NullSpaceDLL::InteropTrackingUpdate t;


		auto quat = update->chest();
		t.chest.w = quat->w();
		t.chest.x = quat->x();
		t.chest.y = quat->y();
		t.chest.z = quat->z();
		return t;

	}


	static PackedPattern FlatbuffDecoder::Decode(const NullSpace::HapticFiles::Mixed::Pattern* pattern, std::unique_ptr<DependencyResolver>& resolver) {
		std::vector<TimeIndex<PackedSequence>> test;

		//std::vector<HapticFrame> frames;
		const auto items = pattern->items();
	//	frames.reserve(items->size());

		for (const auto& frame : *items) {

			auto seq = Decode(frame->sequence(), resolver);
			test.push_back(TimeIndex<PackedSequence>(frame->time(), PackedSequence("placeholder", seq, AreaFlag(frame->area()))));
			//frames.push_back(HapticFrame(frame->time(), seq, AreaFlag(frame->area()), 1));
		}

		return PackedPattern("placeholder", test);
	}
};

