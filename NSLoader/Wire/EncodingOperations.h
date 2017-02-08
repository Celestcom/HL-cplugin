#pragma once
#define NOMINMAX
#include "flatbuffers\flatbuffers.h"

#include "TinyEffect_generated.h"
#include "TinyEffectArray_generated.h"
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
#include <mutex>
#include "Locator.h"
//need to include the quaternion structures a better way
#ifndef IS_ENGINE
#include "NSLoader.h"
#else 
#include "../ExportedStructures.h" //MUST CHANGE TO ExportedStructures
#endif
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
}

typedef std::function<void(uint8_t*, int)> DataCallback;


// This monstrosity is to enforce that the proper encode/finalize methods exist. 
// It does not enfore decoding but we could add it. 
template<
	typename ImuOffset,
	typename ClientStatus,
	typename SuitStatus,
	typename TrackOffset,
	typename HandleCommandOffset,
	typename EngineCommandOffset,
	typename NodeOffset,
typename TinyEffectArrayOffset>

class IEncoder {
	public:
		
		virtual NodeOffset Encode(const Node& input) = 0;
		virtual ImuOffset Encode(bool inpu) = 0;
		virtual TrackOffset Encode(const Quaternion& input) = 0;
		virtual ClientStatus Encode(NullSpace::Communication::Status status) = 0;
		virtual SuitStatus Encode(NullSpace::Communication::SuitStatus status) = 0;
		virtual HandleCommandOffset Encode(NullSpaceDLL::HandleCommand h) = 0;
		virtual EngineCommandOffset Encode(NullSpaceDLL::EngineCommand e) = 0;
		virtual TinyEffectArrayOffset Encode(std::vector<Node*>& input) = 0;

		virtual void Finalize(HandleCommandOffset, std::string, DataCallback) = 0;
		virtual void Finalize(TrackOffset, DataCallback) = 0;
		virtual void Finalize(ImuOffset, DataCallback) = 0;
		virtual void Finalize(uint32_t handle, NodeOffset, std::string, DataCallback) = 0;
		virtual void Finalize(ClientStatus, DataCallback) = 0;
		virtual void Finalize(SuitStatus, DataCallback) = 0;
		virtual void Finalize(EngineCommandOffset, std::string, DataCallback) = 0;
		virtual void Finalize(uint32_t handle, TinyEffectArrayOffset input, DataCallback callback) = 0;
};


typedef struct flatbuffers::Offset<NullSpace::Communication::TrackingUpdate> TrackOffset;
typedef struct flatbuffers::Offset<NullSpace::HapticFiles::Tracking> ImuOffset;
typedef struct flatbuffers::Offset<NullSpace::Communication::SuitStatusUpdate> StatusOffset;
typedef struct flatbuffers::Offset<NullSpace::Communication::ClientStatusUpdate> ClientOffset;
typedef struct flatbuffers::Offset<NullSpace::HapticFiles::HandleCommand> HandleCommandOffset;
typedef struct flatbuffers::Offset<NullSpace::HapticFiles::EngineCommandData> EngineCommandOffset;
typedef struct flatbuffers::Offset<NullSpace::HapticFiles::Node> NodeOffset;
typedef struct flatbuffers::Offset<NullSpace::HapticFiles::TinyEffectArray> TinyEffectArrayOffset;

class EncodingOperations : public IEncoder<ImuOffset, ClientOffset, StatusOffset, TrackOffset, HandleCommandOffset, EngineCommandOffset, NodeOffset, TinyEffectArrayOffset>
{
private:
	flatbuffers::FlatBufferBuilder _builder;
	std::mutex _encodingLock;
public:
	EncodingOperations() {

	}
	void AquireEncodingLock() {
		_encodingLock.lock();
	}
	void ReleaseEncodingLock() {
		_encodingLock.unlock();
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

	EngineCommandOffset EncodingOperations::Encode(NullSpaceDLL::EngineCommand e) {
		return NullSpace::HapticFiles::CreateEngineCommandData(_builder, e.Command);
	}
	TrackOffset EncodingOperations::Encode(const Quaternion& input) {
		//todo: check if this stack variable is copied by flatbuffers
		auto q = NullSpace::Communication::Quaternion(input.x, input.y, input.z, input.w);
		return NullSpace::Communication::CreateTrackingUpdate(_builder, &q);
	}
	
	NodeOffset EncodingOperations::EncodeEffect(const Node& input) {
		auto effect = _builder.CreateString(input.Effect);
		NullSpace::HapticFiles::NodeBuilder nodeBuilder(_builder);
		nodeBuilder.add_effect(effect);
		nodeBuilder.add_duration(input.Duration);
		nodeBuilder.add_strength(input.Strength);
		nodeBuilder.add_time(input.Time);
		nodeBuilder.add_type(NullSpace::HapticFiles::NodeType_Effect);
		return nodeBuilder.Finish();
	}
	NodeOffset EncodingOperations::EncodeSequence(const Node& input) {
		std::vector<NodeOffset> vec;
		for (auto child : input.Children) {
			vec.push_back(Encode(child));
		}

		auto children = _builder.CreateVector(vec);
		auto effect = _builder.CreateString(input.Effect);

		NullSpace::HapticFiles::NodeBuilder nodeBuilder(_builder);
		nodeBuilder.add_area(input.Area);
		nodeBuilder.add_strength(input.Strength);
		nodeBuilder.add_time(input.Time);
		nodeBuilder.add_effect(effect);
		nodeBuilder.add_children(children);
		nodeBuilder.add_type(NullSpace::HapticFiles::NodeType_Sequence);

		return nodeBuilder.Finish();
	}
	NodeOffset EncodingOperations::EncodePattern(const Node& input) {
		std::vector<NodeOffset> vec;
		for (auto child : input.Children) {
			vec.push_back(Encode(child));
		}
		auto children = _builder.CreateVector(vec);
		auto effect = _builder.CreateString(input.Effect);

		NullSpace::HapticFiles::NodeBuilder nodeBuilder(_builder);
		nodeBuilder.add_effect(effect);
		nodeBuilder.add_time(input.Time);
		nodeBuilder.add_children(children);
		nodeBuilder.add_type(NullSpace::HapticFiles::NodeType_Pattern);

		return nodeBuilder.Finish();
	}
	NodeOffset EncodingOperations::EncodeExperience(const Node& input) {
		std::vector<NodeOffset> vec;
		for (auto child : input.Children) {
			vec.push_back(Encode(child));
		}

		auto children = _builder.CreateVector(vec);
		auto effect = _builder.CreateString(input.Effect);

		NullSpace::HapticFiles::NodeBuilder nodeBuilder(_builder);
		nodeBuilder.add_effect(effect);
		nodeBuilder.add_time(input.Time);
		nodeBuilder.add_children(children);
		nodeBuilder.add_type(NullSpace::HapticFiles::NodeType_Experience);
		return nodeBuilder.Finish();
	}
	flatbuffers::Offset<NullSpace::HapticFiles::TinyEffect> EncodingOperations::Encode(const Node* input) {
		
		NullSpace::HapticFiles::TinyEffectBuilder effectBuilder(_builder);
		effectBuilder.add_duration(input->Duration);
		//todo: ADD REAL EFFECT HERE
		effectBuilder.add_effect(Locator::getTranslator().ToEffectFamily(input->Effect));
		effectBuilder.add_strength(input->Strength);
		effectBuilder.add_time(input->Time);
		effectBuilder.add_area(input->Area);
		return effectBuilder.Finish();
	}
	TinyEffectArrayOffset EncodingOperations::Encode(std::vector<Node*>& input) {
		std::vector<flatbuffers::Offset<NullSpace::HapticFiles::TinyEffect>> effects;
		for (const auto& child : input) {
			effects.push_back(Encode(child));
		}
		auto effectsOffset = _builder.CreateVector(effects);

		return NullSpace::HapticFiles::CreateTinyEffectArray(_builder, effectsOffset);


	}
	NodeOffset EncodingOperations::Encode(const Node& input) {
		//Base case
		if (input.Type == Node::EffectType::Effect) {
			return EncodeEffect(input);
		}
		else if (input.Type == Node::EffectType::Sequence) {
			return EncodeSequence(input);
		}
		else if (input.Type == Node::EffectType::Pattern) {
			return EncodePattern(input);
		}
		else if (input.Type == Node::EffectType::Experience) {
			return EncodeExperience(input);
		}
		else {
			//should throw?
			
		}
		
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
	void EncodingOperations::Finalize(uint32_t handle, TinyEffectArrayOffset input, DataCallback callback) {
		auto packet = NullSpace::HapticFiles::CreateHapticPacket(_builder, _builder.CreateString("tinyeffectarray"), NullSpace::HapticFiles::FileType_TinyEffectArray, input.Union(), handle);
		_finalize(packet, callback);
	}
	void EncodingOperations::Finalize(uint32_t handle, NodeOffset input, std::string name, DataCallback callback) {
		auto packet = NullSpace::HapticFiles::CreateHapticPacket(_builder, _builder.CreateString(name), NullSpace::HapticFiles::FileType_Node, input.Union(), handle);
		_finalize(packet, callback);
	}
	void EncodingOperations::Finalize(HandleCommandOffset input, std::string name, DataCallback callback) {
		auto packet = NullSpace::HapticFiles::CreateHapticPacket(_builder, _builder.CreateString("handle"), NullSpace::HapticFiles::FileType::FileType_HandleCommand, input.Union());
		_finalize(packet, callback);
	}
	void EncodingOperations::Finalize(EngineCommandOffset input, std::string name, DataCallback callback) {
		auto packet = NullSpace::HapticFiles::CreateHapticPacket(_builder, _builder.CreateString("engine command"), NullSpace::HapticFiles::FileType::FileType_EngineCommandData, input.Union());
		_finalize(packet, callback);
	}
	void EncodingOperations::Finalize(ImuOffset input, DataCallback callback) {
		auto packet = NullSpace::HapticFiles::CreateHapticPacket(_builder, _builder.CreateString("tracking"), NullSpace::HapticFiles::FileType::FileType_Tracking, input.Union());
		_finalize(packet, callback);
	}


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

	static std::vector<JsonSequenceAtom> EncodingOperations::DecodeSequence(const NullSpace::HapticFiles::Node* node) {
		assert(node->type() == NullSpace::HapticFiles::NodeType_Sequence);
		
		std::vector<JsonSequenceAtom> atoms;
		atoms.reserve(node->children()->size());
		for (const auto& child : *node->children()) {
			assert(child->type() == NullSpace::HapticFiles::NodeType_Effect);
			JsonSequenceAtom a;
			a.Duration = child->duration();
			a.Effect = child->effect()->str();
			a.Repeat = 0;
			a.Strength = child->strength();
			a.Time = child->time();
			atoms.push_back(a);
		}
		return atoms;
	}
	static std::vector<HapticFrame> EncodingOperations::DecodePattern(const NullSpace::HapticFiles::Node* node) {
		assert(node->type() == NullSpace::HapticFiles::NodeType_Pattern);

		std::vector<HapticFrame> atoms;
		atoms.reserve(node->children()->size());
		for (const auto& child : *node->children()) {
			auto x = DecodeSequence(child);
			atoms.push_back(HapticFrame(child->time(), x, AreaFlag(child->area()), 1, child->strength()));


		}
		return atoms;
	}
	static std::vector<HapticSample> EncodingOperations::DecodeExperience(const NullSpace::HapticFiles::Node* node) {
		assert(node->type() == NullSpace::HapticFiles::NodeType_Experience);

		std::vector<HapticSample> atoms;
		atoms.reserve(node->children()->size());
		for (const auto& child : *node->children()) {
			auto x = DecodePattern(child);
			atoms.push_back(HapticSample(child->time(), x, 1));


		}
		return atoms;
	}
	static bool EncodingOperations::Decode(const NullSpace::HapticFiles::Tracking* tracking) {
		return tracking->enable();
	}
	static NullSpaceDLL::EngineCommand EncodingOperations::Decode(const NullSpace::HapticFiles::EngineCommandData* command)
	{
		return NullSpaceDLL::EngineCommand(command->command());
	}
	static NullSpaceDLL::HandleCommand EncodingOperations::Decode(const NullSpace::HapticFiles::HandleCommand* command) {
		return NullSpaceDLL::HandleCommand((unsigned int) command->handle(), command->command());
	}


	
	static std::vector<TinyEffect> EncodingOperations::Decode(const NullSpace::HapticFiles::TinyEffectArray* effects) {
		std::vector<TinyEffect> result;
		result.reserve(effects->effects()->size());
		for (const auto& effect : *effects->effects()) {
			result.push_back(TinyEffect(effect->time(), effect->strength(), effect->duration(), effect->effect(), effect->area()));
		}
		return result;
	}



	static NullSpace::Communication::SuitStatus EncodingOperations::Decode(const NullSpace::Communication::SuitStatusUpdate* update) {
		return update->status();
	}
	static NullSpaceDLL::InteropTrackingUpdate EncodingOperations::Decode(const NullSpace::Communication::TrackingUpdate* update) {
		NullSpaceDLL::InteropTrackingUpdate t;
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

