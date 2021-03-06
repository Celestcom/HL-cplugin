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

HLVR_RETURN(HLVR_Result) HLVR_System_GetRuntimeInfo(const HLVR_System * systemPtr, HLVR_RuntimeInfo * infoPtr)
{
	RETURN_IF_NULL(systemPtr);

	return ExceptionGuard([&] {
		return AS_TYPE(const Engine, systemPtr)->PollStatus(infoPtr);
	});

}






HLVR_RETURN(HLVR_Result) HLVR_DeviceIterator_Init(HLVR_DeviceIterator * iter)
{
	RETURN_IF_NULL(iter);

	iter->_internal = nullptr;
	iter->DeviceInfo = { 0 };
	return HLVR_Ok;
}

HLVR_RETURN(HLVR_Result) HLVR_DeviceIterator_Next(HLVR_DeviceIterator* iter, HLVR_System* system)
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

HLVR_RETURN(int) HLVR_NodeIterator_Next(HLVR_NodeIterator * iter, uint32_t device_id, HLVR_System * system)
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


HLVR_RETURN(HLVR_Result) HLVR_System_Create(HLVR_System** systemPtr)
{
	//todo: return an error if the service version is incompatible. Check registry key? Something?
	return ExceptionGuard([&] { *systemPtr = AS_TYPE(HLVR_System, new Engine()); return HLVR_Ok; });
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

HLVR_RETURN(void) HLVR_System_Destroy(HLVR_System* ptr)
{	
	ExceptionGuard([&] {
		delete AS_TYPE(Engine, ptr);
		return HLVR_Ok;
	});
}



HLVR_RETURN(HLVR_Result) HLVR_System_SuspendEffects(HLVR_System* ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->DoEngineCommand(EngineCommand::PauseAll);
	 });
 }

HLVR_RETURN(HLVR_Result) HLVR_System_ResumeEffects(HLVR_System* ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->DoEngineCommand(EngineCommand::ResumeAll);
	 });
 }

HLVR_RETURN(HLVR_Result) HLVR_System_CancelEffects(HLVR_System* ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->DoEngineCommand(EngineCommand::DestroyAll);
	 });
 }



HLVR_RETURN(HLVR_Result) HLVR_System_PollTracking(HLVR_System * ptr, HLVR_TrackingUpdate * updatePtr)
 {
	 RETURN_IF_NULL(ptr);
	 RETURN_IF_NULL(updatePtr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->PollTracking(updatePtr);

	 });
 }

HLVR_RETURN_EXP(HLVR_Result) HLVR_System_Tracking_GetOrientation(HLVR_System * ptr, uint32_t region, HLVR_Quaternion * outOrientation)
{
	RETURN_IF_NULL(ptr);
	RETURN_IF_NULL(outOrientation);

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, ptr)->GetOrientation(region, outOrientation);
	});
}

HLVR_RETURN_EXP(HLVR_Result) HLVR_System_Tracking_GetCompass(HLVR_System * ptr, uint32_t region, HLVR_Vector3f * outCompass)
{
	RETURN_IF_NULL(ptr);
	RETURN_IF_NULL(outCompass);

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, ptr)->GetCompass(region, outCompass);
	});
}

HLVR_RETURN_EXP(HLVR_Result) HLVR_System_Tracking_GetGravity(HLVR_System * ptr, uint32_t region, HLVR_Vector3f * outGravity)
{
	RETURN_IF_NULL(ptr);
	RETURN_IF_NULL(outGravity);

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, ptr)->GetGravity(region, outGravity);
	});
}

HLVR_RETURN_EXP(HLVR_Result) HLVR_System_Tracking_Enable(HLVR_System * ptr, uint32_t device_id)
{
	RETURN_IF_NULL(ptr);

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, ptr)->EnableTracking(device_id);
	});
}

HLVR_RETURN_EXP(HLVR_Result) HLVR_System_Tracking_Disable(HLVR_System * ptr, uint32_t device_id)
{
	RETURN_IF_NULL(ptr);

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, ptr)->DisableTracking(device_id);
	});
}

HLVR_RETURN_EXP(HLVR_Result) HLVR_System_PushEvent(HLVR_System* system, HLVR_Event * data)
{
	RETURN_IF_NULL(system);
	RETURN_IF_NULL(data);

	return ExceptionGuard([&] { return AS_TYPE(Engine, system)->StreamEvent(*AS_TYPE(TypedEvent, data)); });
}




HLVR_RETURN(HLVR_Result) HLVR_Event_Create(HLVR_Event** eventPtr, HLVR_EventType type)
 {
	
	return ExceptionGuard([&] {
		TypedEvent* e = new TypedEvent(type);
		*eventPtr = AS_TYPE(HLVR_Event, e);

		 RETURN(HLVR_Ok);
	 });
 }

HLVR_RETURN(void) HLVR_Event_Destroy(HLVR_Event * eventPtr)
 {
	ExceptionGuard([&] {
		delete AS_TYPE(TypedEvent, eventPtr);
		return HLVR_Ok;

	});
 }

HLVR_RETURN(HLVR_Result) HLVR_Event_SetFloat(HLVR_Event * event, HLVR_EventKey key, float value)
 {
	 RETURN_IF_NULL(event);

	 return ExceptionGuard([&] {
		 return AS_TYPE(TypedEvent, event)->Params.Set<float>(key, value);
	 });
 }

HLVR_RETURN(HLVR_Result) HLVR_Event_SetFloats(HLVR_Event * event, HLVR_EventKey key, const float* values, unsigned int length)
{
	RETURN_IF_NULL(event);

	return ExceptionGuard([&] {
		
		return AS_TYPE(TypedEvent, event)->Params.Set(key, values, length);
	});
}

HLVR_RETURN(HLVR_Result)HLVR_Event_SetInt(HLVR_Event * event, HLVR_EventKey key, int value)
 {
	 RETURN_IF_NULL(event);

	 return ExceptionGuard([&] {
		 return AS_TYPE(TypedEvent, event)->Params.Set<int>(key, value);
	 });
 }

HLVR_RETURN(HLVR_Result) HLVR_Event_SetInts(HLVR_Event * event, HLVR_EventKey key, const int*  array, unsigned int length)
{
	RETURN_IF_NULL(event);
	return ExceptionGuard([&] {
		return AS_TYPE(TypedEvent, event)->Params.Set(key, array, length);
	});
}

HLVR_RETURN(HLVR_Result) HLVR_Event_SetUInt32(HLVR_Event * event, HLVR_EventKey key, uint32_t value)
{
	RETURN_IF_NULL(event);
	return ExceptionGuard([&] {
		return AS_TYPE(TypedEvent, event)->Params.Set(key, value);
	});
}

HLVR_RETURN(HLVR_Result) HLVR_Event_SetUInt32s(HLVR_Event * event, HLVR_EventKey key, const uint32_t* array, unsigned int length)
{
	RETURN_IF_NULL(event);
	return ExceptionGuard([&] {
		return AS_TYPE(TypedEvent, event)->Params.Set(key, array, length);
	});
}

HLVR_RETURN(HLVR_Result) HLVR_Event_SetUInt64(HLVR_Event * event, HLVR_EventKey key, uint64_t value)
{
	RETURN_IF_NULL(event);

	return ExceptionGuard([&] {
		return AS_TYPE(TypedEvent, event)->Params.Set(key, value);
	});
}

HLVR_RETURN(HLVR_Result) HLVR_Event_SetUInt64s(HLVR_Event * event, HLVR_EventKey key, const uint64_t*  array, unsigned int length)
{
	RETURN_IF_NULL(event);

	return ExceptionGuard([&] {
		return AS_TYPE(TypedEvent, event)->Params.Set(key, array, length);
	});
}




HLVR_RETURN(HLVR_Result) HLVR_Timeline_AddEvent(HLVR_Timeline * timeline, double timeOffsetSeconds, const HLVR_Event * event)
{
	RETURN_IF_NULL(timeline);
	RETURN_IF_NULL(event);

	if (timeOffsetSeconds < 0.0f) {
		RETURN(HLVR_Error_InvalidTimeOffset);
	}

	//note we are taking the time offset as a double and then casting to float
	//if we need the precision later we can have it
	
	return ExceptionGuard([&] {
		return AS_TYPE(EventList, timeline)->AddEvent(
			TimeOffset<TypedEvent> {
				static_cast<float>(timeOffsetSeconds),
				*AS_TYPE(const TypedEvent, event)
			}
		);
	});
}


HLVR_RETURN(HLVR_Result) HLVR_Timeline_Create(HLVR_Timeline** timelinePtr)
 {

	 return ExceptionGuard([&] {

		 *timelinePtr = AS_TYPE(HLVR_Timeline, new EventList());
		 return HLVR_Ok;
	 });
 }

 

HLVR_RETURN(void) HLVR_Timeline_Destroy(HLVR_Timeline * listPtr)
 {
	ExceptionGuard([&] {

		delete AS_TYPE(EventList, listPtr);
		return HLVR_Ok;

	});
 }

HLVR_RETURN(HLVR_Result) HLVR_Event_Validate(const HLVR_Event * event, HLVR_Event_ValidationResult * outResult)
{
	RETURN_IF_NULL(event);
	RETURN_IF_NULL(outResult);

	return ExceptionGuard([&] {
		auto p = PlayableEvent::make(AS_TYPE(const TypedEvent, event)->Type, 0.0f);
		if (!p) {
			RETURN(HLVR_Error_InvalidEventType);
		}
		p->debug_parse(AS_TYPE(const TypedEvent, event)->Params, outResult);
		
		RETURN(HLVR_Ok);
	});
}



HLVR_RETURN(HLVR_Result) HLVR_Timeline_Transmit(const HLVR_Timeline * timelinePtr, HLVR_System* systemPtr, HLVR_Effect * handlePtr)
 {
	 RETURN_IF_NULL(systemPtr);
	 RETURN_IF_NULL(timelinePtr);
	 RETURN_IF_NULL(handlePtr);

	 return ExceptionGuard([&] {

		//Doing a lot of reinterpret_casts here, so I get it out of the way first.
		auto engine = AS_TYPE(Engine, systemPtr);
		auto timeline = AS_TYPE(const EventList, timelinePtr);
		auto handle = AS_TYPE(PlaybackHandle, handlePtr);


		//If this effect was already attached to something playing, we want to release that previously playing thing
		//so that it can eventually be cleaned up. Else it will dangle. 
		if (handle->IsBound()) {
			engine->ReleaseHandle(handle->handle);
		}
		
		EffectHandle newHandle = 0;
		auto retCode = engine->CreateEffect(timeline, &newHandle);
		if (HLVR_OK(retCode)) {
			//Only modify the state of the handle if the engine was able to successfully create the effect
			handle->bind(newHandle, engine);
		}
		return retCode;
		 
	 });
 }



HLVR_RETURN(HLVR_Result) HLVR_Effect_Create(HLVR_Effect ** handlePtr)
 {
	 return ExceptionGuard([&] {
		 *handlePtr = AS_TYPE(HLVR_Effect, new PlaybackHandle());
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



HLVR_RETURN(void) HLVR_Effect_Destroy(HLVR_Effect* handlePtr)
 {
	ExceptionGuard([&] {
		delete AS_TYPE(PlaybackHandle, handlePtr);
		return HLVR_Ok;
	});
 }



HLVR_RETURN(HLVR_Result) HLVR_Effect_GetInfo(const HLVR_Effect* effect, HLVR_EffectInfo* info)
{
	RETURN_IF_NULL(effect);
	RETURN_IF_NULL(info);

	return ExceptionGuard([&] {
		return AS_TYPE(const PlaybackHandle, effect)->GetInfo(info);
	});
}




