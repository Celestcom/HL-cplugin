#include "stdafx.h"
#include "HardlightMk3ZoneDriver.h"
#include "Locator.h"
#include "NSLoader.h"



class BasicHapticEventCreator : public boost::static_visitor<LiveBasicHapticEvent> {
private:
	//Main ID of the effect
	boost::uuids::uuid m_parentId;
	Location m_area;
	boost::uuids::uuid m_uniqueId;
public:
	BasicHapticEventCreator(boost::uuids::uuid id, boost::uuids::uuid unique_id, Location area) : m_parentId(id), m_uniqueId(unique_id), m_area(area) {}

	LiveBasicHapticEvent operator()(const BasicHapticEvent& hapticEvent) const {
		BasicHapticEventData data;
		data.area = static_cast<uint32_t>(m_area);
		data.duration = hapticEvent.duration();
		data.strength = hapticEvent.strength();
		data.effect = hapticEvent.effectFamily();
		return LiveBasicHapticEvent(m_parentId, m_uniqueId, data);
	}
};

CommandBuffer Hardlight_Mk3_ZoneDriver::update(float dt)
{
	//think about if the commandbuffer vectors should really be reversed
	auto rtpCommands = m_rtpModel.Update(dt);
	auto retainedCommands = m_retainedModel.Update(dt);

	std::lock_guard<std::mutex> guard(m_mutex);

	CommandBuffer result;
	result.swap(m_commands);

	if (m_currentMode == Mode::Realtime) {
		result.insert(result.end(), rtpCommands.begin(), rtpCommands.end());
	}
	else {
		result.insert(result.end(), retainedCommands.begin(), retainedCommands.end());
	}

	return result;

}


boost::uuids::uuid Hardlight_Mk3_ZoneDriver::Id() const
{
	return m_parentId;
}


Hardlight_Mk3_ZoneDriver::Hardlight_Mk3_ZoneDriver(::Location area) :
	m_parentId(boost::uuids::random_generator()()),
	m_area(area),
	m_currentMode(Mode::Retained),
	m_commands(),
	m_rtpModel(m_area),
	m_retainedModel(m_area),
	m_mutex()
{

}

::Location Hardlight_Mk3_ZoneDriver::Location()
{
	return static_cast<::Location>(m_area);
}

boost::optional<HapticDisplayInfo> Hardlight_Mk3_ZoneDriver::QueryCurrentlyPlaying()
{
	auto& translator = Locator::getTranslator();

	if (m_currentMode == Mode::Retained) {
		auto potentialEvent = m_retainedModel.GetCurrentlyPlayingEvent();
		if (potentialEvent) {
			HapticDisplayInfo info;
			info.area = translator.ToArea(m_area);
			info.family = potentialEvent->Data().effect;
			info.strength = static_cast<uint16_t>(255 * potentialEvent->Data().strength);
			return info;
		}
	}
	else {
		HapticDisplayInfo info;
		info.area = translator.ToArea(m_area);
		info.family = 0;
		info.strength = m_rtpModel.GetVolume();
		return info;
	}

	return boost::optional<HapticDisplayInfo>();
}

void Hardlight_Mk3_ZoneDriver::realtime(const RealtimeArgs& args)
{
	m_rtpModel.ChangeVolume(args.volume);
	transitionInto(Mode::Realtime);
}

void Hardlight_Mk3_ZoneDriver::transitionInto(Mode mode)
{
	std::lock_guard<std::mutex> guard(m_mutex);
	//Better long term solution: the drivers are notified of disconnects and reconnects so that they can
	//send the commands properly.
	using namespace NullSpaceIPC;
	//if (m_currentMode == Mode::Realtime) {
	if (mode == Mode::Retained) {
		m_currentMode = Mode::Retained;
		//send command to go into retained mode.
		EffectCommand enableRetained;
		enableRetained.set_area(static_cast<uint32_t>(m_area));
		enableRetained.set_command(NullSpaceIPC::EffectCommand_Command_ENABLE_INTRIG);
		m_commands.push_back(enableRetained);
	}
	else
		//	}
		//else {
		if (mode == Mode::Realtime) {
			m_currentMode = Mode::Realtime;
			EffectCommand enableRetained;
			enableRetained.set_area(static_cast<uint32_t>(m_area));
			enableRetained.set_command(NullSpaceIPC::EffectCommand_Command_ENABLE_RTP);
			m_commands.push_back(enableRetained);
		}
	//}
}

void Hardlight_Mk3_ZoneDriver::createRetained(boost::uuids::uuid handle, const SuitEvent& event)
{
	m_retainedModel.Put(boost::apply_visitor(BasicHapticEventCreator(handle, m_gen(), m_area), event));

	transitionInto(Mode::Retained);

}

void Hardlight_Mk3_ZoneDriver::controlRetained(boost::uuids::uuid handle, NSVR_PlaybackCommand command)
{

	switch (command) {
	case NSVR_PlaybackCommand::NSVR_PlaybackCommand_Play:
		m_retainedModel.Play(handle);
		break;
	case NSVR_PlaybackCommand::NSVR_PlaybackCommand_Pause:
		m_retainedModel.Pause(handle);
		break;
	case NSVR_PlaybackCommand::NSVR_PlaybackCommand_Reset:
		m_retainedModel.Remove(handle);
	default:
		break;
	}
}