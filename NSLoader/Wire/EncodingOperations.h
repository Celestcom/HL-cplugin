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

	struct HandleCommand {
		unsigned int Handle;
		NullSpace::HapticFiles::Command Command;
		HandleCommand(unsigned int h, NullSpace::HapticFiles::Command c) :Handle(h), Command(c) {}
		HandleCommand(unsigned int h, short c) : Handle(h), Command(NullSpace::HapticFiles::Command(c)) {}
	};
}

typedef std::function<void(uint8_t*, int)> DataCallback;

template<typename Seq, typename Pat, typename Exp, typename ImuOffset, typename ClientStatus, typename SuitStatus, typename TrackOffset, typename HandleCommandOffset>
class IEncoder {
public:
	virtual Pat Encode(const PackedPattern& input) = 0;
	virtual Seq Encode(const PackedSequence& input) = 0;
	virtual Exp Encode(const PackedExperience& input) = 0;
	virtual ImuOffset Encode(bool inpu) = 0;
	virtual TrackOffset Encode(const Quaternion& input) = 0;
	virtual ClientStatus Encode(NullSpace::Communication::Status status) = 0;
	virtual SuitStatus Encode(NullSpace::Communication::SuitStatus status) = 0;
	virtual HandleCommandOffset Encode(NullSpaceDLL::HandleCommand h) = 0;

	virtual void Finalize(HandleCommandOffset, std::string, DataCallback) = 0;
	virtual void Finalize(TrackOffset, DataCallback) = 0;
	virtual void Finalize(Pat, std::string, DataCallback) = 0;
	virtual void Finalize(unsigned int handle, Seq, std::string, DataCallback) = 0;
	virtual void Finalize(Exp, std::string, DataCallback) = 0;
	virtual void Finalize(ImuOffset, DataCallback) = 0;
	virtual void Finalize(ClientStatus, DataCallback) = 0;
	virtual void Finalize(SuitStatus, DataCallback) = 0;
};

typedef struct flatbuffers::Offset<NullSpace::HapticFiles::Sequence> SeqOffset;
typedef struct flatbuffers::Offset<NullSpace::HapticFiles::Pattern> PatOffset;
typedef struct flatbuffers::Offset<NullSpace::HapticFiles::Experience> ExpOffset;
typedef struct flatbuffers::Offset<NullSpace::Communication::TrackingUpdate> TrackOffset;
typedef struct flatbuffers::Offset<NullSpace::HapticFiles::Tracking> ImuOffset;
typedef struct flatbuffers::Offset<NullSpace::Communication::SuitStatusUpdate> StatusOffset;
typedef struct flatbuffers::Offset<NullSpace::Communication::ClientStatusUpdate> ClientOffset;
typedef struct flatbuffers::Offset<NullSpace::HapticFiles::HandleCommand> HandleCommandOffset;
class EncodingOperations : public IEncoder<SeqOffset, PatOffset, ExpOffset, ImuOffset, ClientOffset, StatusOffset, TrackOffset, HandleCommandOffset>
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
	HandleCommandOffset EncodingOperations::Encode(NullSpaceDLL::HandleCommand h) {
		return NullSpace::HapticFiles::CreateHandleCommand(_builder, h.Handle, h.Command);
	}
	TrackOffset EncodingOperations::Encode(const Quaternion& input) {
		//todo: check if this stack variable is copied by flatbuffers
		auto q = NullSpace::Communication::Quaternion(input.x, input.y, input.z, input.w);
		return NullSpace::Communication::CreateTrackingUpdate(_builder, &q);
	}
	ExpOffset EncodingOperations::Encode(const PackedExperience& input) {
		std::vector<flatbuffers::Offset<NullSpace::HapticFiles::HapticSample>> sample_vector;
		sample_vector.reserve(input.JsonAtoms().size());
		for (auto const &e : input.JsonAtoms()) {
			auto pat = NullSpace::HapticFiles::CreateHapticSample(_builder, e.Time, Encode(e.Haptic));
			
			sample_vector.push_back(pat);
		}
		auto samples = _builder.CreateVector(sample_vector);
		return NullSpace::HapticFiles::CreateExperience(_builder, samples);

	}
	/*
	struct flatbuffers::Offset<NullSpace::HapticFiles::HapticEffect> EncodingOperations::Encode(const HapticEffect& e) {
		

		return  NullSpace::HapticFiles::CreateHapticEffect(
			_builder, (uint16_t)e.Effect, (uint16_t)e.Location, e.Duration, (uint16_t)e.Priority, e.Time);

	}
	*/
	SeqOffset EncodingOperations::Encode(const PackedSequence& input)
	{
		std::vector<flatbuffers::Offset<NullSpace::HapticFiles::HapticEffect>> effects_vector;
		effects_vector.reserve(input.JsonAtoms().size());
		for (auto const &e : input.JsonAtoms()) {
		
			auto effect = NullSpace::HapticFiles::CreateHapticEffect(_builder,e.Time, _builder.CreateString(e.Effect), e.Strength, e.Duration, e.Repeat);
			effects_vector.push_back(effect);
		}
		auto effects = _builder.CreateVector(effects_vector);
		return NullSpace::HapticFiles::CreateSequence(_builder, uint32_t(input.Area()), effects);
	}
	PatOffset EncodingOperations::Encode(const PackedPattern& input)
	{
		std::vector<flatbuffers::Offset<NullSpace::HapticFiles::HapticFrame>> frame_vector;
		frame_vector.reserve(input.PackedAtoms().size());

		for (auto const &f : input.PackedAtoms()) {
			

			auto frame = NullSpace::HapticFiles::CreateHapticFrame(_builder, f.Time, Encode(f.Haptic), uint32_t(f.Haptic.Area()));
			frame_vector.push_back(frame);
		}
		auto frames = _builder.CreateVector(frame_vector);

		return NullSpace::HapticFiles::CreatePattern(_builder, frames);

	}
	ImuOffset EncodingOperations::Encode(bool imusEnabled) {
		return  NullSpace::HapticFiles::CreateTracking(_builder, imusEnabled);
	}
	ClientOffset EncodingOperations::Encode(NullSpace::Communication::Status status) {
		return NullSpace::Communication::CreateClientStatusUpdate(_builder, status);
	}
	StatusOffset EncodingOperations::Encode(NullSpace::Communication::SuitStatus status) {
		return NullSpace::Communication::CreateSuitStatusUpdate(_builder, status);

	}
	
	/* Sending */
	/*****************************************************************/

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
	void EncodingOperations::Finalize(HandleCommandOffset input, std::string name, DataCallback callback) {
		auto packet = NullSpace::HapticFiles::CreateHapticPacket(_builder, _builder.CreateString("handle"), NullSpace::HapticFiles::FileType::FileType_HandleCommand, input.Union());
		_finalize(packet, callback);
	}
	void EncodingOperations::Finalize(ImuOffset input, DataCallback callback) {
		auto packet = NullSpace::HapticFiles::CreateHapticPacket(_builder, _builder.CreateString("tracking"), NullSpace::HapticFiles::FileType::FileType_Tracking, input.Union());
		_finalize(packet, callback);
	}
	void EncodingOperations::Finalize(ExpOffset input, std::string name, DataCallback callback) {
		auto packet = NullSpace::HapticFiles::CreateHapticPacket(_builder, _builder.CreateString(name), NullSpace::HapticFiles::FileType_Experience, input.Union());
		_finalize(packet, callback);
	}	
	void EncodingOperations::Finalize(PatOffset input, std::string name, DataCallback callback)
	{
		auto packet = NullSpace::HapticFiles::CreateHapticPacket(_builder, _builder.CreateString(name), NullSpace::HapticFiles::FileType_Pattern, input.Union());
		_finalize(packet, callback);
	}
	void EncodingOperations::Finalize(unsigned int handle, SeqOffset input, std::string name, DataCallback callback)
	{

		auto packet = NullSpace::HapticFiles::CreateHapticPacket(
			_builder, _builder.CreateString(name) , NullSpace::HapticFiles::FileType_Sequence, input.Union(), handle);
		_finalize(packet, callback);
	}
	/*
	void EncodingOperations::Finalize(struct flatbuffers::Offset<NullSpace::HapticFiles::HapticEffect> input, DataCallback callback) {
		auto packet = NullSpace::HapticFiles::CreateHapticPacket(_builder, _builder.CreateString("REPLACE_ME"), NullSpace::HapticFiles::FileType::FileType_HapticEffect, input.Union());
		_finalize(packet, callback);
	}
	*/
	void EncodingOperations::Finalize(StatusOffset input, DataCallback callback) {
		auto packet = NullSpace::Communication::CreateEnginePacket(_builder, NullSpace::Communication::PacketType::PacketType_SuitStatusUpdate, input.Union());
		_finalize(packet, callback);
	}
	void EncodingOperations::Finalize(TrackOffset input, DataCallback callback)
	{
		auto packet = NullSpace::Communication::CreateEnginePacket(_builder, NullSpace::Communication::PacketType::PacketType_TrackingUpdate, input.Union());
		_finalize(packet, callback);
	}
	void EncodingOperations::Finalize(ClientOffset input, DataCallback callback) {}


	/* Decoding*/
	/*****************************************************************/
	static bool EncodingOperations::Decode(const NullSpace::HapticFiles::Tracking* tracking) {
		return tracking->enable();
	}
	static JsonSequenceAtom EncodingOperations::Decode(const NullSpace::HapticFiles::HapticEffect* effect) {
		return JsonSequenceAtom(effect->time(), effect->effect()->str(), effect->strength(), effect->duration(), effect->repeat());
	}
	static std::vector<JsonSequenceAtom> EncodingOperations::Decode(const NullSpace::HapticFiles::Sequence* sequence) {
		std::vector<JsonSequenceAtom> atoms;
		atoms.reserve(sequence->items()->size());
		auto seqs = sequence->items();
		for (const auto& e : *seqs) {
			atoms.push_back(Decode(e));
		}
		return atoms;
	}

	static NullSpaceDLL::HandleCommand EncodingOperations::Decode(const NullSpace::HapticFiles::HandleCommand* command) {
		return NullSpaceDLL::HandleCommand(command->handle(), command->command());
	}

	
	/*
	static std::vector<HapticEffect> EncodingOperations::Decode(const NullSpace::HapticFiles::Sequence* sequence)
	{
		std::vector<HapticEffect> effects;
		auto items = sequence->items();
		effects.reserve(items->size());

		for (const auto& e : *items) {
			//effects.push_back(HapticEffect(Effect(e->effect()), Location(e->location()), e->duration(), e->time(), e->priority()));
		}

		return effects;
	}
	*/
	/*
	static std::vector<HapticFrame> EncodingOperations::Decode(const NullSpace::HapticFiles::Pattern* pattern)
	{
		std::vector<HapticFrame> frames;
		const auto items = pattern->items();
		frames.reserve(items->size());

		for (const auto& frame : *items) {
			std::vector<HapticSequence> seqs;
			//seqs.reserve(frame->frame()->size());
			//for (const auto& seq : *frame->frame()) {
			//	auto s = Decode(seq);
			//	seqs.push_back(s);
			//}
		//	frames.push_back(HapticFrame(frame->time(), seqs, frame->priority()));
		}

		return frames;
	}
	static std::vector<HapticSample> EncodingOperations::Decode(const NullSpace::HapticFiles::Experience* experience) {
		std::vector<HapticSample> samples;
		auto items = experience->items();
		samples.reserve(items->size());

		for (const auto& sample : *items) {
		//	samples.push_back(HapticSample(sample->time(), Decode(sample->frames()), sample->priority()));

		}
		return samples;
	}
	static HapticEffect EncodingOperations::Decode(const NullSpace::HapticFiles::HapticEffect* effect) {
		return HapticEffect(Effect(0), Location(0), effect->duration(), effect->time(), 0);
		
	}
	*/
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

