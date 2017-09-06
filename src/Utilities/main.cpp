#include "stdafx.h"
#include "utilities.h"

void testEmanations(NSVR_System* system) {
	NSVR_Util_StrikeParams params = { 0 };
	params.Effect = NSVR_Effect_Click;
	params.FromRegion = nsvr_region_lower_arm_left;
	params.ToRegion = nsvr_region_lower_arm_right;
	params.TotalDuration = 0.0;

	NSVR_Timeline* timeline;
	NSVR_Timeline_Create(&timeline);
	//	NSVR_Util_Strike(timeline, &params);

	NSVR_Util_EmanationParams params2 = { 0 };
	params2.Depth = 16;
	params2.Effect = NSVR_Effect_Hum;
	params2.FromRegion = nsvr_region_chest_left;
	params2.FromStrength = 1.0;
	params2.ToStrength = 0.0f;
	params2.TotalDuration = 0.5;
	params2.Gather = true;

	NSVR_Util_Emanation(timeline, &params2);

	NSVR_PlaybackHandle* handle;
	NSVR_PlaybackHandle_Create(&handle);

	NSVR_Timeline_Transmit(timeline, system, handle);

	NSVR_Timeline_Release(&timeline);

	while (true) {
		std::cin.get();
		NSVR_PlaybackHandle_Command(handle, NSVR_PlaybackCommand_Play);
	}

}

void testCurves(NSVR_System* system) {



	NSVR_Timeline* timeline;
	NSVR_Timeline_Create(&timeline);


	NSVR_Event* event;
	NSVR_Event_Create(&event, NSVR_EventType_CurveHapticEvent);
	NSVR_Event_SetFloat(event, "time", 0.0);
	std::vector<float> offsets = { 0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f };
	NSVR_Event_SetFloats(event, "time-offsets", offsets.data(), offsets.size());

	std::vector<float> mags = { 0.0f, 0.3f, 0.5f, 0.8f, 1.0f, 1.0f, 0.8f, 0.7f, 0.6f, 0.4f, 0.0f };

	NSVR_Event_SetFloats(event, "magnitudes", mags.data(), mags.size());
	NSVR_Event_SetUInt32(event, "area", nsvr_region_chest_left);
	NSVR_Timeline_AddEvent(timeline, event);

	NSVR_PlaybackHandle* handle;
	NSVR_PlaybackHandle_Create(&handle);
	NSVR_Timeline_Transmit(timeline, system, handle);



	NSVR_Event_Release(&event);


	NSVR_Timeline_Release(&timeline);

	NSVR_PlaybackHandle_Command(handle, NSVR_PlaybackCommand_Play);
}
int main() {

	NSVR_System* system;
	NSVR_System_Create(&system);

//	testCurves(system);
	testEmanations(system);
	std::cin.get();
	return 0;
}