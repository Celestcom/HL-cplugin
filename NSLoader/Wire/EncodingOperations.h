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
#include "HapticClasses.h"
namespace NullSpaceDLL {
	struct Quaternion {
		float w;
		float x;
		float y;
		float z;
	};
	struct TrackingUpdate {
		Quaternion chest;
		Quaternion left_upper_arm;
		Quaternion left_forearm;
		Quaternion right_upper_arm;
		Quaternion right_forearm;
	};
}

struct Quaternion {
	float w;
	float x;
	float y;
	float z;
	Quaternion(float x, float y, float z, float w) :w(w), x(x), y(y), z(z) {}
	Quaternion() {}
	friend std::ostream &operator<<(std::ostream &output,
		const Quaternion &q) {
		output << "(" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ")";
		return output;
	}
};


class EncodingOperations
{
private:
	flatbuffers::FlatBufferBuilder _builder;
public:
	EncodingOperations() {
	
	}
	static bool VerifyHapticPacket(flatbuffers::FlatBufferBuilder& builder) {
		flatbuffers::Verifier verifier(builder.GetBufferPointer(), builder.GetSize());
		return NullSpace::HapticFiles::VerifyHapticPacketBuffer(verifier);
	}
	static bool VerifyEnginePacket(flatbuffers::FlatBufferBuilder& builder) {
		flatbuffers::Verifier verifier(builder.GetBufferPointer(), builder.GetSize());
		return NullSpace::Communication::VerifyEnginePacketBuffer(verifier);
	}
	
	/* Encoding */
	/*****************************************************************/
	struct flatbuffers::Offset<NullSpace::Communication::TrackingUpdate> EncodingOperations::Encode(const Quaternion& input) {
		//todo: check if this stack variable is copied by flatbuffers
		auto q = NullSpace::Communication::Quaternion(input.x, input.y, input.z, input.w);
		return NullSpace::Communication::CreateTrackingUpdate(_builder, &q);
	}
	struct flatbuffers::Offset<NullSpace::HapticFiles::Experience> EncodingOperations::Encode(const std::vector<HapticSample>& input) {
		std::vector<flatbuffers::Offset<NullSpace::HapticFiles::HapticSample>> sample_vector;
		sample_vector.reserve(input.size());
		for (auto const &e : input) {
			auto sample = NullSpace::HapticFiles::CreateHapticSample(_builder, this->Encode(e.Frames), e.Priority, e.Time);
			sample_vector.push_back(sample);
		}
		auto samples = _builder.CreateVector(sample_vector);
		return NullSpace::HapticFiles::CreateExperience(_builder, samples);

	}
	struct flatbuffers::Offset<NullSpace::HapticFiles::HapticEffect> EncodingOperations::Encode(const HapticEffect& e) {
		

		return  NullSpace::HapticFiles::CreateHapticEffect(
			_builder, (uint16_t)e.Effect, (uint16_t)e.Location, e.Duration, (uint16_t)e.Priority, e.Time);

	}
	struct flatbuffers::Offset<NullSpace::HapticFiles::Sequence> EncodingOperations::Encode(const std::vector<HapticEffect>& input)
	{
		std::vector<flatbuffers::Offset<NullSpace::HapticFiles::HapticEffect>> effects_vector;
		effects_vector.reserve(input.size());
		for (auto const &e : input) {
			auto effect = NullSpace::HapticFiles::CreateHapticEffect(
				_builder, (uint16_t)e.Effect, (uint16_t)e.Location, e.Duration, e.Priority, e.Time);
			effects_vector.push_back(effect);
		}
		auto effects = _builder.CreateVector(effects_vector);
		return NullSpace::HapticFiles::CreateSequence(_builder, effects);
	}
	struct flatbuffers::Offset<NullSpace::HapticFiles::Pattern> EncodingOperations::Encode(const std::vector<HapticFrame>& input)
	{
		std::vector<flatbuffers::Offset<NullSpace::HapticFiles::HapticFrame>> frame_vector;
		frame_vector.reserve(input.size());

		for (auto const &f : input) {
			std::vector<flatbuffers::Offset<NullSpace::HapticFiles::Sequence>> sequence_vector;
			for (auto const &seq : f.Frame) {
				sequence_vector.push_back(this->Encode(seq.Effects));
			}
			auto frames = _builder.CreateVector(sequence_vector);
			auto frame = NullSpace::HapticFiles::CreateHapticFrame(_builder, frames, f.Priority, f.Time);
			frame_vector.push_back(frame);
		}
		auto frames = _builder.CreateVector(frame_vector);

		return NullSpace::HapticFiles::CreatePattern(_builder, frames);

	}
	struct flatbuffers::Offset<NullSpace::HapticFiles::Tracking> EncodingOperations::Encode(bool imusEnabled) {
		return  NullSpace::HapticFiles::CreateTracking(_builder, imusEnabled);
	}
	struct flatbuffers::Offset<NullSpace::Communication::ClientStatusUpdate> EncodingOperations::Encode(NullSpace::Communication::Status status) {
		return NullSpace::Communication::CreateClientStatusUpdate(_builder, status);
	}
	struct flatbuffers::Offset<NullSpace::Communication::SuitStatusUpdate> EncodingOperations::Encode(NullSpace::Communication::SuitStatus status) {
		return NullSpace::Communication::CreateSuitStatusUpdate(_builder, status);

	}
	
	/* Sending */
	/*****************************************************************/
	typedef std::function<void(uint8_t*, int)> DataCallback;

	void EncodingOperations::_finalize(struct flatbuffers::Offset<NullSpace::HapticFiles::HapticPacket> input, std::function<void(uint8_t*, int)> callback) {
		
		_builder.Finish(input);
		if (VerifyHapticPacket(_builder)) {
			callback(_builder.GetBufferPointer(), _builder.GetSize());
		}
		_builder.Clear();
	}
	void EncodingOperations::_finalize(struct flatbuffers::Offset<NullSpace::Communication::EnginePacket> input, std::function<void(uint8_t*, int)> callback) {

		_builder.Finish(input);
		if (VerifyEnginePacket(_builder)) {
			callback(_builder.GetBufferPointer(), _builder.GetSize());
		}
		_builder.Clear();
	}

	void EncodingOperations::Finalize(struct flatbuffers::Offset<NullSpace::HapticFiles::Tracking> input, DataCallback callback) {
		auto packet = NullSpace::HapticFiles::CreateHapticPacket(_builder, _builder.CreateString("tracking"), NullSpace::HapticFiles::FileType::FileType_Tracking, input.Union());
		_finalize(packet, callback);
	}
	void EncodingOperations::Finalize(struct flatbuffers::Offset<NullSpace::HapticFiles::Experience> input, std::string name, DataCallback callback) {
		auto packet = NullSpace::HapticFiles::CreateHapticPacket(_builder, _builder.CreateString(name), NullSpace::HapticFiles::FileType_Experience, input.Union());
		_finalize(packet, callback);
	}	
	void EncodingOperations::Finalize(struct flatbuffers::Offset<NullSpace::HapticFiles::Pattern> input, std::string name, DataCallback callback)
	{
		auto packet = NullSpace::HapticFiles::CreateHapticPacket(_builder, _builder.CreateString(name), NullSpace::HapticFiles::FileType_Pattern, input.Union());
		_finalize(packet, callback);
	}
	void EncodingOperations::Finalize(struct flatbuffers::Offset<NullSpace::HapticFiles::Sequence> input, std::string name, DataCallback callback)
	{

		auto packet = NullSpace::HapticFiles::CreateHapticPacket(_builder, _builder.CreateString(name) , NullSpace::HapticFiles::FileType_Sequence, input.Union());
		_finalize(packet, callback);
	}
	void EncodingOperations::Finalize(struct flatbuffers::Offset<NullSpace::HapticFiles::HapticEffect> input, DataCallback callback) {
		auto packet = NullSpace::HapticFiles::CreateHapticPacket(_builder, _builder.CreateString("REPLACE_ME"), NullSpace::HapticFiles::FileType::FileType_HapticEffect, input.Union());
		_finalize(packet, callback);
	}
	void EncodingOperations::Finalize(struct flatbuffers::Offset<NullSpace::Communication::SuitStatusUpdate> input, DataCallback callback) {
		auto packet = NullSpace::Communication::CreateEnginePacket(_builder, NullSpace::Communication::PacketType::PacketType_SuitStatusUpdate, input.Union());
		_finalize(packet, callback);
	}
	void EncodingOperations::Finalize(struct flatbuffers::Offset<NullSpace::Communication::TrackingUpdate> input, DataCallback callback)
	{
		auto packet = NullSpace::Communication::CreateEnginePacket(_builder, NullSpace::Communication::PacketType::PacketType_TrackingUpdate, input.Union());
		_finalize(packet, callback);
	}


	/* Decoding*/
	/*****************************************************************/
	static bool EncodingOperations::Decode(const NullSpace::HapticFiles::Tracking* tracking) {
		return tracking->enable();
	}
	static std::vector<HapticEffect> EncodingOperations::Decode(const NullSpace::HapticFiles::Sequence* sequence)
	{
		std::vector<HapticEffect> effects;
		auto items = sequence->items();
		effects.reserve(items->size());

		for (const auto& e : *items) {
			effects.push_back(HapticEffect(Effect(e->effect()), Location(e->location()), e->duration(), e->time(), e->priority()));
		}

		return effects;
	}
	static std::vector<HapticFrame> EncodingOperations::Decode(const NullSpace::HapticFiles::Pattern* pattern)
	{
		std::vector<HapticFrame> frames;
		const auto items = pattern->items();
		frames.reserve(items->size());

		for (const auto& frame : *items) {
			std::vector<HapticSequence> seqs;
			seqs.reserve(frame->frame()->size());
			for (const auto& seq : *frame->frame()) {
				auto s = Decode(seq);
				seqs.push_back(s);
			}
			frames.push_back(HapticFrame(frame->time(), seqs, frame->priority()));
		}

		return frames;
	}
	static std::vector<HapticSample> EncodingOperations::Decode(const NullSpace::HapticFiles::Experience* experience) {
		std::vector<HapticSample> samples;
		auto items = experience->items();
		samples.reserve(items->size());

		for (const auto& sample : *items) {
			samples.push_back(HapticSample(sample->time(), Decode(sample->frames()), sample->priority()));

		}
		return samples;
	}
	static HapticEffect EncodingOperations::Decode(const NullSpace::HapticFiles::HapticEffect* effect) {
		return HapticEffect(Effect(effect->effect()), Location(effect->location()), effect->duration(), effect->time(), effect->priority());
		
	}
	static NullSpace::Communication::SuitStatus EncodingOperations::Decode(const NullSpace::Communication::SuitStatusUpdate* update) {
		return update->status();
	}
	static NullSpaceDLL::TrackingUpdate EncodingOperations::Decode(const NullSpace::Communication::TrackingUpdate* update) {
		NullSpaceDLL::TrackingUpdate t;

		
		auto quat = update->chest();
		t.chest.w = quat->w();
		t.chest.x = quat->x();
		t.chest.y = quat->y();
		t.chest.z = quat->z();
		return t;

	}
	
	~EncodingOperations() {
		
	}
};

