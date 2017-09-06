#include "stdafx.h"

#include "Engine.h"
#include "ExceptionSafeCall.h"
#include "BodyView.h"
#define AS_TYPE(Type, Obj) reinterpret_cast<Type *>(Obj)
#define AS_CTYPE(Type, Obj) reinterpret_cast<const Type *>(Obj)


//The null stuff should be put in one place probably
//comment this line if you want to disable argument null checking. Think really hard before doing this.
#define NULL_ARGUMENT_CHECKS



#ifdef NULL_ARGUMENT_CHECKS
#define RETURN_IF_NULL(ptr) do { if (ptr == nullptr) { return (NSVR_Result) NSVR_Error_NullArgument; }} while (0)
#else
#define RETURN_IF_NULL(ptr)
#endif


NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_System_PollLogs(NSVR_System * system, NSVR_LogEntry * entry)
{
	RETURN_IF_NULL(system);
	RETURN_IF_NULL(entry);

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, system)->PollLogs(entry);
	});
}

NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_System_GetStats(NSVR_System * system, NSVR_SystemStats * ptr)
{
	RETURN_IF_NULL(system);
	RETURN_IF_NULL(ptr);

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, system)->GetEngineStats(ptr);
	});
}

NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_System_Audio_Enable(NSVR_System * system, NSVR_AudioOptions* optionsPtr)
{
	RETURN_IF_NULL(system);

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, system)->EnableAudio(optionsPtr);
	});
}

NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_System_Audio_Disable(NSVR_System * system)
{
	RETURN_IF_NULL(system);
	return ExceptionGuard([&] {
		return AS_TYPE(Engine, system)->DisableAudio();
	});
}

NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_System_SubmitRawCommand(NSVR_System * system, uint8_t * buffer, int length)
{
	RETURN_IF_NULL(system);
	RETURN_IF_NULL(buffer);



	return ExceptionGuard([&] {
		return AS_TYPE(Engine, system)->SubmitRawCommand(buffer, length);
	});
}

NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_System_DumpDeviceDiagnostics(NSVR_System* systemPtr)
{
	RETURN_IF_NULL(systemPtr);
	
	return ExceptionGuard([&] {
		return AS_TYPE(Engine, systemPtr)->DumpDeviceDiagnostics();
	});
}






NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_Immediate_Set(NSVR_System * systemPtr, uint32_t * regions, double * amplitudes, uint32_t length)
{
	RETURN_IF_NULL(systemPtr);
	RETURN_IF_NULL(regions);
	RETURN_IF_NULL(amplitudes);

	if (length < 0) {
		return NSVR_Error_InvalidArgument;
	}

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, systemPtr)->SetStrengths(regions, amplitudes, length);
	});
}

NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_BodyView_Create(NSVR_BodyView** body)
{
	return ExceptionGuard([&] {
		*body = AS_TYPE(NSVR_BodyView, new BodyView());
		return NSVR_Success_Unqualified;
	});
}

NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_BodyView_Release(NSVR_BodyView ** body)
{
	return ExceptionGuard([&] {
		delete AS_TYPE(BodyView, *body);
		*body = nullptr;
		return NSVR_Success_Unqualified;
	});
}

NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_BodyView_Poll(NSVR_BodyView * body, NSVR_System * system)
{
	RETURN_IF_NULL(system);
	RETURN_IF_NULL(body);

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, system)->UpdateView(AS_TYPE(BodyView, body));
	});
}

NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_BodyView_GetNodeCount(NSVR_BodyView * body, uint32_t * outNodeCount)
{
	RETURN_IF_NULL(body);
	RETURN_IF_NULL(outNodeCount);

	return ExceptionGuard([&] {
		//todo: be defensive about overflow?
		*outNodeCount = static_cast<uint32_t>(AS_TYPE(BodyView, body)->pairs.size());
		return NSVR_Success_Unqualified;
	});
}

NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_BodyView_GetNodeType(NSVR_BodyView * body, uint32_t nodeIndex, uint32_t * outType)
{
	RETURN_IF_NULL(body);
	RETURN_IF_NULL(outType);
	return ExceptionGuard([&] {
		return AS_TYPE(BodyView, body)->getNodeType(nodeIndex, outType);
	});
}

NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_BodyView_GetNodeRegion(NSVR_BodyView * body, uint32_t nodeIndex, uint32_t * outRegion)
{

	RETURN_IF_NULL(body);
	RETURN_IF_NULL(outRegion);
	return ExceptionGuard([&] {
		return AS_TYPE(BodyView, body)->getNodeRegion(nodeIndex, outRegion);
	});
}

NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_BodyView_GetIntensity(NSVR_BodyView * body, uint32_t nodeIndex, float * outIntensity)
{

	RETURN_IF_NULL(body);
	RETURN_IF_NULL(outIntensity);

	return ExceptionGuard([&] {
		return AS_TYPE(BodyView, body)->getIntensity(nodeIndex, outIntensity);
	});
}

NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_BodyView_GetColor(NSVR_BodyView * body, uint32_t nodeIndex, NSVR_Color * outColor)
{

	RETURN_IF_NULL(body);
	RETURN_IF_NULL(outColor);

	return ExceptionGuard([&] {
		return AS_TYPE(BodyView, body)->getNodeColor(nodeIndex, outColor);
	});
}
