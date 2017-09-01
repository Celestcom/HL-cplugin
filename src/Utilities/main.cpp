#include "stdafx.h"
#include "utilities.h"
int main() {

	NSVR_System* system;
	NSVR_System_Create(&system);


	NSVR_Util_StrikeParams params = { 0 };
	params.Effect = NSVR_Effect_Click;
	params.FromRegion = nsvr_region_chest_left;
	params.ToRegion = nsvr_region_lower_ab_right;
	params.TotalDuration = 4.0;

	NSVR_Timeline* timeline;
	NSVR_Timeline_Create(&timeline);
	NSVR_Util_Strike(timeline, &params);



	NSVR_PlaybackHandle* handle;
	NSVR_PlaybackHandle_Create(&handle);

	NSVR_Timeline_Transmit(timeline, system, handle);

	NSVR_Timeline_Release(&timeline);



	NSVR_PlaybackHandle_Command(handle, NSVR_PlaybackCommand_Play);

	while (true) {
		NSVR_EffectInfo info = { 0 };
		NSVR_PlaybackHandle_GetInfo(handle, &info);
		std::cout << info.Elapsed << '\n';
		if (info.PlaybackState == NSVR_EffectInfo_State_Idle) {
			NSVR_PlaybackHandle_Command(handle, NSVR_PlaybackCommand_Play);
		}

	}
	std::cin.get();
	return 0;
}