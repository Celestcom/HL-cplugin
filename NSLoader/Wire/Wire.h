#pragma once
#include <memory>
#include "zmq.hpp"
#include "zmq_addon.hpp"
#define FLATBUFFERS_DEBUG_VERIFICATION_FAILURE
#include "flatbuffers\flatbuffers.h"
#include "HapticEffect_generated.h"
#include "HapticFrame_generated.h"
#include "HapticPacket_generated.h"

class Wire
{
public:
	struct flatbuffers::Offset<NullSpace::HapticFiles::Sequence> Wire::Encode(const std::vector<HapticEffect>& input)
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

	struct flatbuffers::Offset<NullSpace::HapticFiles::Pattern> Wire::Encode(const std::vector<HapticFrame>& input)
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

	void Wire::Send(const struct flatbuffers::Offset<NullSpace::HapticFiles::Pattern>& input)

	{

		auto packet = NullSpace::HapticFiles::CreateHapticPacket(_builder, _builder.CreateString("placeholder"), NullSpace::HapticFiles::FileType_Pattern, input.Union());
		_builder.Finish(packet);
		const int size = _builder.GetSize();


		flatbuffers::Verifier verifier(_builder.GetBufferPointer(), _builder.GetSize());
		if (NullSpace::HapticFiles::VerifyHapticPacketBuffer(verifier)) {
			zmq::message_t msg(size);
			memcpy((void*)msg.data(), _builder.GetBufferPointer(), size);
			_socket->send(msg);

		}


		_builder.Clear();

	}
	void Wire::Send(const struct flatbuffers::Offset<NullSpace::HapticFiles::Sequence>& input)

	{
		auto packet = NullSpace::HapticFiles::CreateHapticPacket(_builder, _builder.CreateString("placeholder"), NullSpace::HapticFiles::FileType_Sequence, input.Union());
		_builder.Finish(packet);
		const int size = _builder.GetSize();

		flatbuffers::Verifier verifier(_builder.GetBufferPointer(), _builder.GetSize());
		if (NullSpace::HapticFiles::VerifyHapticPacketBuffer(verifier)) {
			zmq::message_t msg(size);
			memcpy((void*)msg.data(), _builder.GetBufferPointer(), size);
			_socket->send(msg);
			
		}




		_builder.Clear();
	}

	static std::vector<HapticEffect> Wire::Decode(const NullSpace::HapticFiles::Sequence* sequence)
	{
		std::vector<HapticEffect> effects;
		auto items = sequence->items();
		effects.reserve(items->size());

		for (const auto& e : *items) {
			effects.push_back(HapticEffect(Effect(e->effect()), Location(e->location()), e->duration(), e->time(), e->priority()));
		}

		return effects;
	}

	static std::vector<HapticFrame> Wire::Decode(const NullSpace::HapticFiles::Pattern* pattern)
	{
		std::vector<HapticFrame> frames;
		auto items = pattern->items();
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
	~Wire()
	{

	}
	Wire(std::string address)
	{
		_context = std::make_unique<zmq::context_t>(1);
		_socket = std::make_unique<zmq::socket_t>(*_context, ZMQ_PAIR);
		_socket->connect(address);
	}

private:
	flatbuffers::FlatBufferBuilder _builder;
	std::unique_ptr<zmq::socket_t> _socket;
	std::unique_ptr<zmq::context_t> _context;
};





