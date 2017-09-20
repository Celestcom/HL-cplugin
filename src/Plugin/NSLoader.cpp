// NSLoader.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NSLoader.h"
#include "Engine.h"
#include "EventList.h"
#include "ParameterizedEvent.h"
#include "PlaybackHandle.h"
#include "ExceptionSafeCall.h"
#include "EngineCommand.h"

#define AS_TYPE(Type, Obj) reinterpret_cast<Type *>(Obj)
#define AS_CTYPE(Type, Obj) reinterpret_cast<const Type *>(Obj)


//comment this line if you want to disable argument null checking. Profile really hard before doing this.
#define NULL_ARGUMENT_CHECKS




#ifdef NULL_ARGUMENT_CHECKS
#define RETURN_IF_NULL(ptr) do { if (ptr == nullptr) { return (NSVR_Result) NSVR_Error_NullArgument; }} while (0)
#define RETURN_FALSE_IF_NULL(ptr) do {if (ptr == nullptr) { return false;}} while (0)
#else
#define RETURN_IF_NULL(ptr)
#define RETURN_FALSE_IF_NULL(ptr)
#endif



NSVR_RETURN(unsigned int) NSVR_Version_Get(void)
{
	return NSLOADER_API_VERSION;
}

NSVR_RETURN(int) NSVR_Version_IsCompatibleDLL(void)
{
	unsigned int major = NSVR_Version_Get() >> 16;
	return major == NSLOADER_API_VERSION_MAJOR;
}

NSVR_RETURN(NSVR_Result) NSVR_System_GetServiceInfo(NSVR_System * systemPtr, NSVR_ServiceInfo * infoPtr)
{
	RETURN_IF_NULL(systemPtr);

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, systemPtr)->PollStatus(infoPtr);
	});

}






NSVR_RETURN(NSVR_Result) NSVR_DeviceInfo_Iter_Init(NSVR_DeviceInfo_Iter * iter)
{
	iter->_internal = nullptr;
	iter->DeviceInfo = { 0 };
	return NSVR_Success_Unqualified;
}

NSVR_RETURN(bool) NSVR_DeviceInfo_Iter_Next(NSVR_DeviceInfo_Iter* iter, NSVR_System* system)
{

	RETURN_FALSE_IF_NULL(iter);
	RETURN_FALSE_IF_NULL(system);
	return ExceptionGuard([iter, system]() {
		if (iter->_internal == nullptr) {
			HiddenIterator<NSVR_DeviceInfo>* snapshot = AS_TYPE(Engine, system)->TakeDeviceSnapshot();
			iter->_internal = snapshot;
		}


		HiddenIterator<NSVR_DeviceInfo>* iterator = AS_TYPE(HiddenIterator<NSVR_DeviceInfo>, iter->_internal);

		if (iterator->Finished()) {
			AS_TYPE(Engine, system)->DestroyIterator(iterator);
			iter->_internal = nullptr;
			return false;
		}

		iterator->NextItem(&iter->DeviceInfo);

		return true;
	});
}

NSVR_RETURN(NSVR_Result) NSVR_NodeInfo_Iter_Init(NSVR_NodeInfo_Iter * iter)
{
	iter->_internal = nullptr;
	iter->NodeInfo = { 0 };
	return NSVR_Success_Unqualified;
}

NSVR_RETURN(bool) NSVR_NodeInfo_Iter_Next(NSVR_NodeInfo_Iter * iter, NSVR_System * system)
{
	RETURN_FALSE_IF_NULL(iter);
	RETURN_FALSE_IF_NULL(system);

	return ExceptionGuard([iter, system]() {

		if (iter->_internal == nullptr) {
			HiddenIterator<NSVR_NodeInfo>* snapshot = AS_TYPE(Engine, system)->TakeNodeSnapshot();
			iter->_internal = snapshot;
		}

		HiddenIterator<NSVR_NodeInfo>* iterator = AS_TYPE(HiddenIterator<NSVR_NodeInfo>, iter->_internal);
		if (iterator->Finished()) {
			AS_TYPE(Engine, system)->DestroyIterator(iterator);
			iter->_internal = nullptr;
			return false;
		}

		iterator->NextItem(&iter->NodeInfo);
		return true;
	});
}


NSVR_RETURN(NSVR_Result) NSVR_System_Create(NSVR_System** systemPtr)
{

	return ExceptionGuard([&] { *systemPtr = AS_TYPE(NSVR_System, new Engine()); return NSVR_Success_Unqualified; });
}

NSVR_RETURN(int) NSVR_Version_HasFeature(const char * feature)
{
	static std::set<std::string> features;

	if (features.empty())
	{
		//cache the feature list
		//features.insert("TRACKING");
		//features.insert("STREAMING");
		//whatever
	}

	return features.find(feature) != features.end();
}

NSVR_RETURN(void) NSVR_System_Release(NSVR_System** ptr)
{	
	ExceptionGuard([&] {
		delete AS_TYPE(Engine, *ptr);
		*ptr = nullptr;

		return NSVR_Success_Unqualified;
	});
}



NSVR_RETURN(NSVR_Result) NSVR_System_Haptics_Suspend(NSVR_System* ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->DoEngineCommand(EngineCommand::PauseAll);
	 });
 }

NSVR_RETURN(NSVR_Result) NSVR_System_Haptics_Resume(NSVR_System* ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->DoEngineCommand(EngineCommand::ResumeAll);
	 });
 }

NSVR_RETURN(NSVR_Result) NSVR_System_Haptics_Destroy(NSVR_System* ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->DoEngineCommand(EngineCommand::DestroyAll);
	 });
 }



NSVR_RETURN(NSVR_Result) NSVR_System_Tracking_Poll(NSVR_System * ptr, NSVR_TrackingUpdate * updatePtr)
 {
	 RETURN_IF_NULL(ptr);
	 RETURN_IF_NULL(updatePtr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->PollTracking(updatePtr);

	 });
 }

NSVR_RETURN(NSVR_Result) NSVR_System_Tracking_Enable(NSVR_System * ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->DoEngineCommand(EngineCommand::EnableTracking);
	 });
 }

NSVR_RETURN(NSVR_Result) NSVR_System_Tracking_Disable(NSVR_System * ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->DoEngineCommand(EngineCommand::DisableTracking);
	 });
 }



NSVR_RETURN(NSVR_Result) NSVR_Event_Create(NSVR_Event** eventPtr, NSVR_EventType type)
 {
	
	return ExceptionGuard([&] {

	
		*eventPtr = AS_TYPE(NSVR_Event, new ParameterizedEvent(type));
		
		BOOST_LOG_TRIVIAL(info) << std::this_thread::get_id() << 
			"[Event " << *eventPtr << "] Create ";

		 return (NSVR_Result) NSVR_Success_Unqualified;
	 });
 }

NSVR_RETURN(void) NSVR_Event_Release(NSVR_Event ** eventPtr)
 {
	ExceptionGuard([&] {
		//BOOST_LOG_TRIVIAL(info) << std::this_thread::get_id() 
		//	<<"[Event " << *eventPtr << "] Release ";

		delete AS_TYPE(ParameterizedEvent, *eventPtr);
		*eventPtr = nullptr;

		return NSVR_Success_Unqualified;
	});
 }

NSVR_RETURN(NSVR_Result) NSVR_Event_SetFloat(NSVR_Event * event, NSVR_EventKey key, float value)
 {
	 RETURN_IF_NULL(event);

	 return ExceptionGuard([&] {
		 return AS_TYPE(ParameterizedEvent, event)->Set<float>(key, value);
	 });
 }

NSVR_RETURN(NSVR_Result) NSVR_Event_SetFloats(NSVR_Event * event, NSVR_EventKey key, float * values, unsigned int length)
{
	RETURN_IF_NULL(event);

	return ExceptionGuard([&] {
		
		return AS_TYPE(ParameterizedEvent, event)->SetFloats(key, values, length);
	});
}

NSVR_RETURN(NSVR_Result)NSVR_Event_SetInt(NSVR_Event * event, NSVR_EventKey key, int value)
 {
	 RETURN_IF_NULL(event);

	 return ExceptionGuard([&] {
		 return AS_TYPE(ParameterizedEvent, event)->Set<int>(key, value);
	 });
 }

NSVR_RETURN(NSVR_Result) NSVR_Event_SetUInt32(NSVR_Event * event, NSVR_EventKey key, uint32_t value)
{
	RETURN_IF_NULL(event);
	return ExceptionGuard([&] {
		return AS_TYPE(ParameterizedEvent, event)->Set<uint32_t>(key, value);
	});
}

NSVR_RETURN(NSVR_Result) NSVR_Event_SetUInt32s(NSVR_Event * event, NSVR_EventKey key, uint32_t* array, unsigned int length)
{
	RETURN_IF_NULL(event);
	return ExceptionGuard([&] {
		return AS_TYPE(ParameterizedEvent, event)->SetUInt32s(key, array, length);
	});
}


NSVR_RETURN(NSVR_Result)NSVR_Timeline_Create(NSVR_Timeline** timelinePtr)
 {

	 return ExceptionGuard([&] {

		 *timelinePtr = AS_TYPE(NSVR_Timeline, new EventList());
		// BOOST_LOG_TRIVIAL(info) << std::this_thread::get_id() <<
		//	 "[Timeline " << *timelinePtr << "] Create";

		 return NSVR_Success_Unqualified;
	 });
 }

 

NSVR_RETURN(void) NSVR_Timeline_Release(NSVR_Timeline ** listPtr)
 {
	ExceptionGuard([&] {
		//BOOST_LOG_TRIVIAL(info) << std::this_thread::get_id() <<
		//	"[Timeline " << *listPtr << "] Release";

		delete AS_TYPE(EventList, *listPtr);
		*listPtr = nullptr;

		return NSVR_Success_Unqualified;
	});
 }

NSVR_RETURN(NSVR_Result) NSVR_Timeline_AddEvent(NSVR_Timeline * list, NSVR_Event * event)
 {
	 RETURN_IF_NULL(list);
	 RETURN_IF_NULL(event);

	 return ExceptionGuard([&] {
		// BOOST_LOG_TRIVIAL(info) << std::this_thread::get_id() << 
		//	 "[Timeline " << list << "] AddEvent " << event;

		return AS_TYPE(EventList, list)->AddEvent(AS_TYPE(ParameterizedEvent, event));
	 });
 }

NSVR_RETURN(NSVR_Result) NSVR_Timeline_Transmit(NSVR_Timeline * timelinePtr, NSVR_System* systemPtr, NSVR_PlaybackHandle * handlePtr)
 {
	 RETURN_IF_NULL(systemPtr);
	 RETURN_IF_NULL(timelinePtr);
	 RETURN_IF_NULL(handlePtr);

	 return ExceptionGuard([&] {

		 auto engine = AS_TYPE(Engine, systemPtr);
		 auto timeline = AS_TYPE(EventList, timelinePtr);
		 auto handle = AS_TYPE(PlaybackHandle, handlePtr);

		
		 if (handle->handle != 0) {
			 engine->ReleaseHandle(handle->handle);
		 }
		
		 handle->engine = engine;
		 return engine->CreateEffect(timeline, &handle->handle);
		 
	 });
 }

NSVR_RETURN(NSVR_Result) NSVR_Timeline_Combine(NSVR_Timeline * timeline, NSVR_Timeline * mixin, float offset)
{

	///Ya know what nahhh

	//This is complicated
	//We need a case for a == b == c

	// and a == b

	//it should prolly just be add(source, mixin)


	RETURN_IF_NULL(timeline);
	RETURN_IF_NULL(mixin);

	if (timeline != mixin) {
		AS_TYPE(EventList, timeline)->Interleave(AS_TYPE(EventList, mixin), offset);
		return NSVR_Success_Unqualified;
	}
	else {
		AS_TYPE(EventList, timeline)->Dupe(offset);
		return NSVR_Success_Unqualified;

	}
	

	return NSVR_Error_Unknown;
}


NSVR_RETURN(NSVR_Result) NSVR_PlaybackHandle_Create(NSVR_PlaybackHandle ** handlePtr)
 {

	 return ExceptionGuard([&] {
		 *handlePtr = AS_TYPE(NSVR_PlaybackHandle, new PlaybackHandle());
		 BOOST_LOG_TRIVIAL(info) << std::this_thread::get_id() <<
			 "[Handle " << *handlePtr << "] Create";

		 return NSVR_Success_Unqualified;
	 });

	 
}



NSVR_RETURN(NSVR_Result)NSVR_PlaybackHandle_Command(NSVR_PlaybackHandle * handlePtr, NSVR_PlaybackCommand command)
 {
	 RETURN_IF_NULL(handlePtr);

	 return ExceptionGuard([&] {
		 BOOST_LOG_TRIVIAL(info) << std::this_thread::get_id() <<
			 "[Handle " << handlePtr << "] Command " << command;
		return AS_TYPE(PlaybackHandle, handlePtr)->Command(command);
	 });
 }

NSVR_RETURN(void) NSVR_PlaybackHandle_Release(NSVR_PlaybackHandle** handlePtr)
 {
	ExceptionGuard([&] {
		BOOST_LOG_TRIVIAL(info) << std::this_thread::get_id() <<
			"[Handle " << *handlePtr << "] Release";
		delete AS_TYPE(PlaybackHandle, *handlePtr);
		*handlePtr = nullptr;

		return NSVR_Success_Unqualified;
	});
 }



NSVR_RETURN(NSVR_Result) NSVR_PlaybackHandle_GetInfo(NSVR_PlaybackHandle* handlePtr, NSVR_EffectInfo* infoPtr)
{
	RETURN_IF_NULL(handlePtr);
	RETURN_IF_NULL(infoPtr);

	return ExceptionGuard([&] {
		return AS_TYPE(PlaybackHandle, handlePtr)->GetHandleInfo(infoPtr);
	});
}




