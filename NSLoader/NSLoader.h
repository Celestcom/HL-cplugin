#pragma once
#include <stdint.h>

#ifdef NSLOADER_EXPORTS
#define NSLOADER_API __declspec(dllexport) 
#else
#define NSLOADER_API __declspec(dllimport) 
#endif

#define NSLOADER_API_VERSION_MAJOR 0
#define NSLOADER_API_VERSION_MINOR 3
#define NSLOADER_API_VERSION ((NSLOADER_API_VERSION_MAJOR << 16) | NSLOADER_API_VERSION_MINOR)




#ifdef __cplusplus
extern "C" {
#endif

	struct NSVR_Context;;
	typedef struct NSVR_Context* NSVR_Context_t;

	struct NSVR_Quaternion {
		float w;
		float x;
		float y;
		float z;
	};
	struct NSVR_InteropTrackingUpdate {
		NSVR_Quaternion chest;
		NSVR_Quaternion left_upper_arm;
		NSVR_Quaternion left_forearm;
		NSVR_Quaternion right_upper_arm;
		NSVR_Quaternion right_forearm;
	};

	NSLOADER_API unsigned int __stdcall NSVR_GetVersion(void);
	NSLOADER_API  int __stdcall NSVR_IsCompatibleDLL(void);

	NSLOADER_API NSVR_Context_t* __stdcall NSVR_Create();
	NSLOADER_API void __stdcall NSVR_Delete(NSVR_Context_t* ptr);

	NSLOADER_API unsigned int __stdcall NSVR_GenHandle(NSVR_Context_t* ptr);
	NSLOADER_API int  __stdcall NSVR_PollStatus(NSVR_Context_t* ptr);

	NSLOADER_API void __stdcall NSVR_PollTracking(NSVR_Context_t* ptr, NSVR_InteropTrackingUpdate& q);
	

	NSLOADER_API void __stdcall NSVR_HandleCommand(NSVR_Context_t* ptr, uint32_t handle, uint16_t command);

	NSLOADER_API char* __stdcall NSVR_GetError(NSVR_Context_t* ptr);

	NSLOADER_API void __stdcall NSVR_FreeError(char* string);

	NSLOADER_API int __stdcall NSVR_EngineCommand(NSVR_Context_t* ptr, uint16_t command);

	NSLOADER_API int __stdcall NSVR_InitializeFromFilesystem(NSVR_Context_t* ptr, char* path);


	NSLOADER_API uint32_t __stdcall NSVR_TransmitEvents(NSVR_Context_t* ptr, void* data, uint32_t size);
#ifdef __cplusplus
}
#endif

