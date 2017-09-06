#pragma once
#include <string>
#include <iostream>

#include "IoService.h"
#include "ClientMessenger.h"
#include <boost\asio\deadline_timer.hpp>
#include "HapticsPlayer.h"
#include "ScheduledEvent.h"
#include "EventList.h"
#include "NSLoader.h"
#include "MyTestLog.h"
#include "IHapticDevice.h"
#include "NSLoader_Internal.h"
#include "EngineIsAlive.h"
#include "EngineCommand.h"


//The point of this is to enable the user to have something like
// NSVR_Struct_Thing widget = {0};
// while (NSVR_HasNext(&widget)) {
//	//do things with widget
// }
// The HiddenIterator sticks itself into the void* _internal variable of the struct. 

template<typename T>
class HiddenIterator {
public:
	HiddenIterator(std::vector<T> items) : m_items(items) { m_currentItem = m_items.begin(); }
	void NextItem(T* original) {
		*original = *m_currentItem;
		original->_internal = this;
		m_currentItem++;
	}
	bool Finished() {
		return m_currentItem == m_items.end();
	}
private:
	std::vector<T> m_items;
	typename std::vector<T>::iterator m_currentItem;

	
};

class SnapshotContainer {

};
struct BodyView;
class Engine
{

public:
	Engine();


	~Engine();
	int PollStatus(NSVR_ServiceInfo*);
	uint32_t GenerateHandle();
	int PollDevice(NSVR_DeviceInfo *);
	bool DoEngineCommand(::EngineCommand command);
	int GetEngineStats(NSVR_SystemStats* stats);
	void HandleCommand(unsigned int handle, NSVR_PlaybackCommand);
	void ReleaseHandle(unsigned int handle);
	int CreateEffect(EventList * list, HapticHandle * handle);
	int  PollTracking(NSVR_TrackingUpdate* q);

	int PollLogs(NSVR_LogEntry* entry);

	int EnableAudio(NSVR_AudioOptions* options);
	int DisableAudio();

	int SubmitRawCommand(uint8_t* buffer, int length);

	int Sample(uint16_t* strengths, uint32_t* areas, uint32_t* families, int length, unsigned int* resultCount);

	int DumpDeviceDiagnostics();
	int SetStrengths(uint32_t* regions, double* amplitudes, uint32_t length);
	int GetHandleInfo(uint32_t m_handle, NSVR_EffectInfo* infoPtr);

	int GetDevices(NSVR_DeviceInfo* array, uint32_t inLength, uint32_t* outArrayLength);
	int GetNumDevices(uint32_t* outAmount);

	HiddenIterator<NSVR_DeviceInfo>* TakeDeviceSnapshot();

	template<typename T>
	bool IsFinishedIterating(HiddenIterator<T>* param1) const;
private:
	IoService m_ioService;
	NSVR_TrackingUpdate m_cachedTrackingUpdate;
	uint32_t m_currentHandleId;
	bool m_isHapticsSystemPlaying;
	ClientMessenger m_messenger;
	EventRegistry m_registry;

	std::unique_ptr<IHapticDevice> m_hardlightSuit;
	HapticsPlayer m_player;

	boost::posix_time::milliseconds m_hapticsExecutionInterval;
	void executeTimestep(std::chrono::milliseconds dt);

	ScheduledEvent m_hapticsTimestep;

	std::chrono::time_point<std::chrono::steady_clock> m_lastHapticsTimestep;

	boost::shared_ptr<MyTestLog> m_log;

	std::vector<std::unique_ptr<HiddenIterator<NSVR_DeviceInfo>>> m_snapshots;


	void setupUserFacingLogSink();

	void setupFileLogSink();
public:
	int UpdateView(BodyView* view);
	void DestroyIterator(HiddenIterator<NSVR_DeviceInfo>* device);
};

template<typename T>
bool Engine::IsFinishedIterating(HiddenIterator<T>* param1) const
{
	return param1->currentItem == param1->items.end();
//	auto it =std::find_if(m_snapshots.begin(), m_snapshots.end(), [snapshot](const auto& snapshotPtr) { return snapshotPtr.get() == snapshot; });
	//	if (it != m_snapshots.end()) {
	//		return (*it)->currentItem == (*it)->systems.end();
	//	}
	//	return true;
}

