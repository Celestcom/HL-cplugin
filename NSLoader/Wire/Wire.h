#pragma once
#include <memory>
#include "zmq.hpp"
#include "zmq_addon.hpp"
#include "Locator.h"
#define FLATBUFFERS_DEBUG_VERIFICATION_FAILURE
#include "flatbuffers\flatbuffers.h"
#include "HapticEffect_generated.h"
#include "HapticFrame_generated.h"
#include "HapticPacket_generated.h"
#include "HapticSample_generated.h"
#include "ClientStatusUpdate_generated.h"
#include "Experience_generated.h"
#include "EnginePacket_generated.h"
#include "SuitStatusUpdate_generated.h"
#include "HapticClasses.h"
#include "EncodingOperations.h"
class Wire
{
public:
	EncodingOperations Encoder;

	static void Wire::sendTo(zmq::socket_t& socket, uint8_t* data, int size) {
		zmq::message_t msg(size);
		memcpy((void*)msg.data(), data, size);
		socket.send(msg);
	}
	void Wire::sendToEngine(uint8_t* data, int size) {
		sendTo(*_sendToEngineSocket, data, size);
	}
	/* Sending */
	void Wire::Send( struct flatbuffers::Offset<NullSpace::HapticFiles::Experience>& input, std::string name) {
		Encoder.Finalize(input, name, boost::bind(&Wire::sendToEngine, this, _1, _2));
	}
	
	void Wire::Send( struct flatbuffers::Offset<NullSpace::HapticFiles::Pattern>& input, std::string name)
	{
		Encoder.Finalize(input, name, boost::bind(&Wire::sendToEngine, this, _1, _2));
	}

	void Wire::Send( struct flatbuffers::Offset<NullSpace::HapticFiles::Sequence>& input, std::string name)
	{
	
		Encoder.Finalize(input, name, boost::bind(&Wire::sendToEngine, this, _1, _2));
	}

	void Wire::Send( struct flatbuffers::Offset<NullSpace::HapticFiles::HapticEffect> input) {
		Encoder.Finalize(input, boost::bind(&Wire::sendToEngine, this, _1, _2));
		
	}
	void Wire::Send(struct flatbuffers::Offset<NullSpace::Communication::ClientStatusUpdate>& input) {
		
	}
	void Wire::Send( struct flatbuffers::Offset<NullSpace::Communication::SuitStatusUpdate>& input) {
	}
	

	bool Wire::Receive(NullSpace::Communication::SuitStatus& status, NullSpaceDLL::TrackingUpdate& tracking) {
		zmq::message_t msg;
		if (_receiveFromEngineSocket->recv(&msg, ZMQ_DONTWAIT)) {
			flatbuffers::Verifier verifier(reinterpret_cast<uint8_t*>(msg.data()), msg.size());
			if (NullSpace::Communication::VerifyEnginePacketBuffer(verifier)) {
				auto packet = NullSpace::Communication::GetEnginePacket(msg.data());
				if (packet->packet_type() == NullSpace::Communication::PacketType::PacketType_SuitStatusUpdate) {
					auto decoded = EncodingOperations::Decode(static_cast<const NullSpace::Communication::SuitStatusUpdate*>(packet->packet()));
					status = decoded;
					return true;
				}
				else if (packet->packet_type() == NullSpace::Communication::PacketType::PacketType_TrackingUpdate) {
					auto decoded = EncodingOperations::Decode(static_cast<const NullSpace::Communication::TrackingUpdate*>(packet->packet()));
					tracking = decoded;
					return true;
				}

			}
		}
		return false;
	}
	
	
	~Wire()
	{
		/* VERY IMPORTANT. If you do not close the sockets, the DLL will cause Unity to freeze!*/
		_sendToEngineSocket->close();
		_receiveFromEngineSocket->close();
	}
	Wire(std::string sendAddress, std::string receiveAddress)
	{
		_context = std::make_unique<zmq::context_t>(1);
		_sendToEngineSocket = std::make_unique<zmq::socket_t>(*_context, ZMQ_PAIR);
		_sendToEngineSocket->setsockopt(ZMQ_SNDHWM, 1);
		_sendToEngineSocket->connect(sendAddress);
		_sendToEngineSocket->setsockopt(ZMQ_LINGER, 0);
		_receiveFromEngineSocket = std::make_unique<zmq::socket_t>(*_context, ZMQ_SUB);
		
		_receiveFromEngineSocket->setsockopt(ZMQ_CONFLATE, 1);

		_receiveFromEngineSocket->connect(receiveAddress);
		
		_receiveFromEngineSocket->setsockopt(ZMQ_SUBSCRIBE, "", 0);


	}

private:
	std::unique_ptr<zmq::socket_t> _sendToEngineSocket;
	std::unique_ptr<zmq::socket_t> _receiveFromEngineSocket;
	std::unique_ptr<zmq::context_t> _context;
};

