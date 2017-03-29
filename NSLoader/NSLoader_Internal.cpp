#include "stdafx.h"
#include "NSLoader_Internal.h"




#include "NSLoader.h"
#include "Engine.h"
#define AS_TYPE(Type, Obj) reinterpret_cast<Type *>(Obj)
#define AS_CTYPE(Type, Obj) reinterpret_cast<const Type *>(Obj)


NSLOADER_INTERNAL_API NSVR_Result __stdcall NSVR_System_PollLogs(NSVR_System * system, NSVR_LogEntry * entry)
{
	return AS_TYPE(Engine, system)->PollLogs(entry);
}
