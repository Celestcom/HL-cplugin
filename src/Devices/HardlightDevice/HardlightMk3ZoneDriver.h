#pragma once

#include "Enums.h"
#include "IHapticDevice.h"

#include "ZoneModel.h"
#include "RtpModel.h"
#include <boost/uuid/random_generator.hpp>
#include "NSLoader_fwds.h"




class Hardlight_Mk3_ZoneDriver : public HardwareDriver {
public:
	CommandBuffer update(float dt);

	virtual boost::uuids::uuid Id() const override;
	//this should really take a Location probably..
	Hardlight_Mk3_ZoneDriver(Location area);

	Location Location();

	boost::optional<HapticDisplayInfo> QueryCurrentlyPlaying();
private:
	::Location m_area;

	ZoneModel m_retainedModel;
	RtpModel m_rtpModel;
	void createRetained(boost::uuids::uuid handle, const SuitEvent& event) override;
	void controlRetained(boost::uuids::uuid handle, NSVR_PlaybackCommand command) override;
	void realtime(const RealtimeArgs& args) override;

	boost::uuids::random_generator m_gen;
	enum class Mode {Retained, Realtime};
	void transitionInto(Mode mode);
	Mode m_currentMode;
	boost::uuids::uuid m_parentId;
	std::mutex m_mutex;
	CommandBuffer m_commands;
};