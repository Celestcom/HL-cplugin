#include "stdafx.h"
#include "HLVR_Experimental.h"
#include "Engine.h"
#include "ExceptionSafeCall.h"
#include "BodyView.h"
#define AS_TYPE(Type, Obj) reinterpret_cast<Type *>(Obj)
#define AS_CTYPE(Type, Obj) reinterpret_cast<const Type *>(Obj)


//The null stuff should be put in one place probably
//comment this line if you want to disable argument null checking. Think really hard before doing this.
#define NULL_ARGUMENT_CHECKS



#ifdef NULL_ARGUMENT_CHECKS
#define RETURN_IF_NULL(ptr) do { if (ptr == nullptr) { return (HLVR_Result) HLVR_Error_NullArgument; }} while (0)
#else
#define RETURN_IF_NULL(ptr)
#endif







HLVR_RETURN_EXP(HLVR_Result) HLVR_Immediate_Set(HLVR_System * systemPtr, uint32_t * regions, double * amplitudes, uint32_t length)
{
	RETURN_IF_NULL(systemPtr);
	RETURN_IF_NULL(regions);
	RETURN_IF_NULL(amplitudes);

	if (length < 0) {
		return HLVR_Error_InvalidArgument;
	}

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, systemPtr)->SetStrengths(regions, amplitudes, length);
	});
}

HLVR_RETURN_EXP(HLVR_Result) HLVR_BodyView_Create(HLVR_BodyView** body)
{
	return ExceptionGuard([&] {
		*body = AS_TYPE(HLVR_BodyView, new BodyView());
		return HLVR_Ok;
	});
}

HLVR_RETURN_EXP(HLVR_Result) HLVR_BodyView_Release(HLVR_BodyView ** body)
{
	return ExceptionGuard([&] {
		delete AS_TYPE(BodyView, *body);
		*body = nullptr;
		return HLVR_Ok;
	});
}

HLVR_RETURN_EXP(HLVR_Result) HLVR_BodyView_Poll(HLVR_BodyView * body, HLVR_System * system)
{
	RETURN_IF_NULL(system);
	RETURN_IF_NULL(body);

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, system)->UpdateView(AS_TYPE(BodyView, body));
	});
}

HLVR_RETURN_EXP(HLVR_Result) HLVR_BodyView_GetNodeCount(HLVR_BodyView * body, uint32_t * outNodeCount)
{
	RETURN_IF_NULL(body);
	RETURN_IF_NULL(outNodeCount);

	return ExceptionGuard([&] {
		//todo: be defensive about overflow?
		*outNodeCount = static_cast<uint32_t>(AS_TYPE(BodyView, body)->pairs.size());
		return HLVR_Ok;
	});
}

HLVR_RETURN_EXP(HLVR_Result) HLVR_BodyView_GetNodeType(HLVR_BodyView * body, uint32_t nodeIndex, uint32_t * outType)
{
	RETURN_IF_NULL(body);
	RETURN_IF_NULL(outType);
	return ExceptionGuard([&] {
		return AS_TYPE(BodyView, body)->getNodeType(nodeIndex, outType);
	});
}

HLVR_RETURN_EXP(HLVR_Result) HLVR_BodyView_GetNodeRegion(HLVR_BodyView * body, uint32_t nodeIndex, uint32_t * outRegion)
{

	RETURN_IF_NULL(body);
	RETURN_IF_NULL(outRegion);
	return ExceptionGuard([&] {
		return AS_TYPE(BodyView, body)->getNodeRegion(nodeIndex, outRegion);
	});
}

HLVR_RETURN_EXP(HLVR_Result) HLVR_BodyView_GetIntensity(HLVR_BodyView * body, uint32_t nodeIndex, float * outIntensity)
{

	RETURN_IF_NULL(body);
	RETURN_IF_NULL(outIntensity);

	return ExceptionGuard([&] {
		return AS_TYPE(BodyView, body)->getIntensity(nodeIndex, outIntensity);
	});
}

