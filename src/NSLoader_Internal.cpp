#include "stdafx.h"

#include "Engine.h"
#include "ExceptionSafeCall.h"

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


NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_Immediate_Sample(NSVR_System* systemPtr, uint16_t* strengths, uint32_t* areas, uint32_t* families, int length, unsigned int* resultCount)
{
	RETURN_IF_NULL(systemPtr);
	RETURN_IF_NULL(strengths);
	RETURN_IF_NULL(areas);
	RETURN_IF_NULL(resultCount);

	if (length < 16) {
		return NSVR_Error_InvalidArgument;
	}

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, systemPtr)->Sample(strengths, areas, families, length, resultCount);
	});

}

NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_Immediate_Set(NSVR_System* systemPtr, uint16_t* strengths, uint32_t* areas, int length)
{
	RETURN_IF_NULL(systemPtr);
	RETURN_IF_NULL(strengths);
	RETURN_IF_NULL(areas);

	if (length < 0) {
		return NSVR_Error_InvalidArgument;
	}

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, systemPtr)->SetStrengths(strengths, areas, length);
	});

}
