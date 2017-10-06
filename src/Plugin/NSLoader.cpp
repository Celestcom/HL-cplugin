// NSLoader.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "HLVR.h"
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

#define RETURN(HLVR_Result_Value) return static_cast<HLVR_Result>(HLVR_Result_Value);



#ifdef NULL_ARGUMENT_CHECKS
#define RETURN_IF_NULL(ptr) do { if (ptr == nullptr) { return (HLVR_Result) HLVR_Error_NullArgument; }} while (0)
#define RETURN_FALSE_IF_NULL(ptr) do {if (ptr == nullptr) { return false;}} while (0)
#else
#define RETURN_IF_NULL(ptr)
#define RETURN_FALSE_IF_NULL(ptr)
#endif



HLVR_RETURN(uint32_t) HLVR_Version_Get(void)
{
	return HLVR_API_VERSION;
}

HLVR_RETURN(int) HLVR_Version_IsCompatibleDLL(void)
{
	unsigned int major = HLVR_Version_Get() >> 24;
	return major == HLVR_API_VERSION_MAJOR;
}

HLVR_RETURN(HLVR_Result) HLVR_Agent_GetPlatformInfo(HLVR_Agent * systemPtr, HLVR_PlatformInfo * infoPtr)
{
	RETURN_IF_NULL(systemPtr);

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, systemPtr)->PollStatus(infoPtr);
	});

}






HLVR_RETURN(HLVR_Result) HLVR_DeviceIterator_Init(HLVR_DeviceIterator * iter)
{
	iter->_internal = nullptr;
	iter->DeviceInfo = { 0 };
	return HLVR_Ok;
}

HLVR_RETURN(HLVR_Result) HLVR_DeviceIterator_Next(HLVR_DeviceIterator* iter, HLVR_Agent* system)
{

	RETURN_FALSE_IF_NULL(iter);
	RETURN_FALSE_IF_NULL(system);
	return ExceptionGuard([iter, system]() {
		if (iter->_internal == nullptr) {
			HiddenIterator<HLVR_DeviceInfo>* snapshot = AS_TYPE(Engine, system)->TakeDeviceSnapshot();
			iter->_internal = snapshot;
		}


		HiddenIterator<HLVR_DeviceInfo>* iterator = AS_TYPE(HiddenIterator<HLVR_DeviceInfo>, iter->_internal);

		if (iterator->Finished()) {
			AS_TYPE(Engine, system)->DestroyIterator(iterator);
			iter->_internal = nullptr;
			RETURN(HLVR_Error_NoMoreDevices);
		}

		iterator->NextItem(&iter->DeviceInfo);

		RETURN(HLVR_Ok);
	});
}

HLVR_RETURN(HLVR_Result) HLVR_NodeIterator_Init(HLVR_NodeIterator * iter)
{
	iter->_internal = nullptr;
	iter->NodeInfo = { 0 };
	return HLVR_Ok;
}

HLVR_RETURN(int) HLVR_NodeIterator_Next(HLVR_NodeIterator * iter, uint32_t device_id, HLVR_Agent * system)
{
	RETURN_FALSE_IF_NULL(iter);
	RETURN_FALSE_IF_NULL(system);

	return ExceptionGuard([iter, system, device_id]() {

		if (iter->_internal == nullptr) {
			HiddenIterator<HLVR_NodeInfo>* snapshot = AS_TYPE(Engine, system)->TakeNodeSnapshot(device_id);
			iter->_internal = snapshot;
		}

		HiddenIterator<HLVR_NodeInfo>* iterator = AS_TYPE(HiddenIterator<HLVR_NodeInfo>, iter->_internal);
		if (iterator->Finished()) {
			AS_TYPE(Engine, system)->DestroyIterator(iterator);
			iter->_internal = nullptr;
			RETURN(HLVR_Error_NoMoreNodes);
		}

		iterator->NextItem(&iter->NodeInfo);
		RETURN(HLVR_Ok);
	});
}


HLVR_RETURN(HLVR_Result) HLVR_Agent_Create(HLVR_Agent** systemPtr, HLVR_AgentConfiguration* config)
{
	return ExceptionGuard([&] { *systemPtr = AS_TYPE(HLVR_Agent, new Engine()); return HLVR_Ok; });
}

HLVR_RETURN(int) HLVR_Version_HasFeature(const char * feature)
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

HLVR_RETURN(void) HLVR_Agent_Destroy(HLVR_Agent** ptr)
{	
	ExceptionGuard([&] {
		delete AS_TYPE(Engine, *ptr);
		*ptr = nullptr;

		return HLVR_Ok;
	});
}



HLVR_RETURN(HLVR_Result) HLVR_Agent_SuspendEffects(HLVR_Agent* ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->DoEngineCommand(EngineCommand::PauseAll);
	 });
 }

HLVR_RETURN(HLVR_Result) HLVR_Agent_ResumeEffects(HLVR_Agent* ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->DoEngineCommand(EngineCommand::ResumeAll);
	 });
 }

HLVR_RETURN(HLVR_Result) HLVR_Agent_CancelEffects(HLVR_Agent* ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->DoEngineCommand(EngineCommand::DestroyAll);
	 });
 }



HLVR_RETURN(HLVR_Result) HLVR_Agent_PollTracking(HLVR_Agent * ptr, HLVR_TrackingUpdate * updatePtr)
 {
	 RETURN_IF_NULL(ptr);
	 RETURN_IF_NULL(updatePtr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->PollTracking(updatePtr);

	 });
 }

HLVR_RETURN(HLVR_Result) HLVR_Agent_EnableTracking(HLVR_Agent * ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->DoEngineCommand(EngineCommand::EnableTracking);
	 });
 }

HLVR_RETURN(HLVR_Result) HLVR_Agent_DisableTracking(HLVR_Agent * ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->DoEngineCommand(EngineCommand::DisableTracking);
	 });
 }



HLVR_RETURN(HLVR_Result) HLVR_Event_Create(HLVR_Event** eventPtr, HLVR_EventType type)
 {
	
	return ExceptionGuard([&] {

	
		*eventPtr = AS_TYPE(HLVR_Event, new ParameterizedEvent(type));
		
		BOOST_LOG_TRIVIAL(info) << std::this_thread::get_id() << 
			"[Event " << *eventPtr << "] Create ";

		 return (HLVR_Result) HLVR_Ok;
	 });
 }

HLVR_RETURN(void) HLVR_Event_Destroy(HLVR_Event ** eventPtr)
 {
	ExceptionGuard([&] {
		delete AS_TYPE(ParameterizedEvent, *eventPtr);
		*eventPtr = nullptr;
		return HLVR_Ok;

	});
 }

HLVR_RETURN(HLVR_Result) HLVR_Event_SetFloat(HLVR_Event * event, HLVR_EventKey key, float value)
 {
	 RETURN_IF_NULL(event);

	 return ExceptionGuard([&] {
		 return AS_TYPE(ParameterizedEvent, event)->Set<float>(key, value);
	 });
 }

HLVR_RETURN(HLVR_Result) HLVR_Event_SetFloats(HLVR_Event * event, HLVR_EventKey key, float * values, unsigned int length)
{
	RETURN_IF_NULL(event);

	return ExceptionGuard([&] {
		
		return AS_TYPE(ParameterizedEvent, event)->Set(key, values, length);
	});
}

HLVR_RETURN(HLVR_Result)HLVR_Event_SetInt(HLVR_Event * event, HLVR_EventKey key, int value)
 {
	 RETURN_IF_NULL(event);

	 return ExceptionGuard([&] {
		 return AS_TYPE(ParameterizedEvent, event)->Set<int>(key, value);
	 });
 }

HLVR_RETURN(HLVR_Result) HLVR_Event_SetInts(HLVR_Event * event, HLVR_EventKey key, int * array, unsigned int length)
{
	RETURN_IF_NULL(event);
	return ExceptionGuard([&] {
		return AS_TYPE(ParameterizedEvent, event)->Set(key, array, length);
	});
}

HLVR_RETURN(HLVR_Result) HLVR_Event_SetUInt32(HLVR_Event * event, HLVR_EventKey key, uint32_t value)
{
	RETURN_IF_NULL(event);
	return ExceptionGuard([&] {
		return AS_TYPE(ParameterizedEvent, event)->Set(key, value);
	});
}

HLVR_RETURN(HLVR_Result) HLVR_Event_SetUInt32s(HLVR_Event * event, HLVR_EventKey key, uint32_t* array, unsigned int length)
{
	RETURN_IF_NULL(event);
	return ExceptionGuard([&] {
		return AS_TYPE(ParameterizedEvent, event)->Set(key, array, length);
	});
}

HLVR_RETURN(HLVR_Result) HLVR_Event_SetUInt64(HLVR_Event * event, HLVR_EventKey key, uint64_t value)
{
	RETURN_IF_NULL(event);

	return ExceptionGuard([&] {
		return AS_TYPE(ParameterizedEvent, event)->Set(key, value);
	});
}

HLVR_RETURN(HLVR_Result) HLVR_Event_SetUInt64s(HLVR_Event * event, HLVR_EventKey key, uint64_t * array, unsigned int length)
{
	RETURN_IF_NULL(event);

	return ExceptionGuard([&] {
		return AS_TYPE(ParameterizedEvent, event)->Set(key, array, length);
	});
}


HLVR_RETURN(HLVR_Result) HLVR_Timeline_Create(HLVR_Timeline** timelinePtr)
 {

	 return ExceptionGuard([&] {

		 *timelinePtr = AS_TYPE(HLVR_Timeline, new EventList());
		 return HLVR_Ok;
	 });
 }

 

HLVR_RETURN(void) HLVR_Timeline_Destroy(HLVR_Timeline ** listPtr)
 {
	ExceptionGuard([&] {

		delete AS_TYPE(EventList, *listPtr);
		*listPtr = nullptr;
		return HLVR_Ok;

	});
 }

HLVR_RETURN(HLVR_Result) HLVR_Timeline_AddEvent(HLVR_Timeline * list, HLVR_Event * event)
 {
	 RETURN_IF_NULL(list);
	 RETURN_IF_NULL(event);

	 return ExceptionGuard([&] {
		return AS_TYPE(EventList, list)->AddEvent(AS_TYPE(ParameterizedEvent, event));
	 });
 }

HLVR_RETURN(HLVR_Result) HLVR_Timeline_Transmit(HLVR_Timeline * timelinePtr, HLVR_Agent* systemPtr, HLVR_Effect * handlePtr)
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

//HLVR_RETURN(HLVR_Result) HLVR_Timeline_Combine(HLVR_Timeline * timeline, HLVR_Timeline * mixin, float offset)
//{
//
//
//
//	RETURN_IF_NULL(timeline);
//	RETURN_IF_NULL(mixin);
//
//	if (timeline != mixin) {
//		AS_TYPE(EventList, timeline)->Interleave(AS_TYPE(EventList, mixin), offset);
//		return NSVR_Success_Unqualified;
//	}
//	else {
//		AS_TYPE(EventList, timeline)->Dupe(offset);
//		return NSVR_Success_Unqualified;
//
//	}
//	
//
//	return NSVR_Error_Unknown;
//}


HLVR_RETURN(HLVR_Result) HLVR_Effect_Create(HLVR_Effect ** handlePtr)
 {

	 return ExceptionGuard([&] {
		 *handlePtr = AS_TYPE(HLVR_Effect, new PlaybackHandle());
		 BOOST_LOG_TRIVIAL(info) << std::this_thread::get_id() <<
			 "[Handle " << *handlePtr << "] Create";

		 return HLVR_Ok;
	 });

	 
}

HLVR_RETURN(HLVR_Result) HLVR_Effect_Pause(HLVR_Effect* effect) {
	return ExceptionGuard([effect] {
		return AS_TYPE(PlaybackHandle, effect)->Pause();
	});
}

HLVR_RETURN(HLVR_Result) HLVR_Effect_Reset(HLVR_Effect* effect) {
	return ExceptionGuard([effect] {
		return AS_TYPE(PlaybackHandle, effect)->Reset();
	});
}

HLVR_RETURN(HLVR_Result) HLVR_Effect_Play(HLVR_Effect* effect) {
	return ExceptionGuard([effect] {
		return AS_TYPE(PlaybackHandle, effect)->Play();
	});
}



HLVR_RETURN(void) HLVR_Effect_Destroy(HLVR_Effect** handlePtr)
 {
	ExceptionGuard([&] {
		delete AS_TYPE(PlaybackHandle, *handlePtr);
		*handlePtr = nullptr;
		return HLVR_Ok;
	});
 }



HLVR_RETURN(HLVR_Result) HLVR_Effect_GetInfo(HLVR_Effect* effect, HLVR_EffectInfo* info)
{
	RETURN_IF_NULL(effect);
	RETURN_IF_NULL(info);

	return ExceptionGuard([&] {
		return AS_TYPE(PlaybackHandle, effect)->GetHandleInfo(info);
	});
}




