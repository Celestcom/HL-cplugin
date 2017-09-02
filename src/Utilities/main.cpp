#include "stdafx.h"
#include "utilities.h"
int main() {

	NSVR_System* system;
	NSVR_System_Create(&system);


	NSVR_Util_StrikeParams params = { 0 };
	params.Effect = NSVR_Effect_Click;
	params.FromRegion = nsvr_region_lower_arm_left;
	params.ToRegion = nsvr_region_lower_arm_right;
	params.TotalDuration = 0.0;

	NSVR_Timeline* timeline;
	NSVR_Timeline_Create(&timeline);
//	NSVR_Util_Strike(timeline, &params);

	NSVR_Util_EmanationParams params2 = { 0 };
	params2.Depth = 4;
	params2.Effect = NSVR_Effect_Buzz;
	params2.FromRegion = nsvr_region_shoulder_right;
	params2.FromStrength = 1.0;
	params2.ToStrength = 1.0f;
	params2.TotalDuration = 4.0;


	NSVR_Util_Emanation(timeline, &params2);

	NSVR_Timeline* mixin1;
	NSVR_Timeline_Create(&mixin1);

	params2.Effect = NSVR_Effect_Double_Click;
	NSVR_Util_Emanation(mixin1, &params2);

	NSVR_Timeline_Combine(timeline, mixin1, 0.2f);

	NSVR_PlaybackHandle* handle;
	NSVR_PlaybackHandle_Create(&handle);

	NSVR_Timeline_Transmit(timeline, system, handle);

	NSVR_Timeline_Release(&timeline);

	while (true) {
		std::cin.get();
		NSVR_PlaybackHandle_Command(handle, NSVR_PlaybackCommand_Play);
	}

	std::cin.get();
	return 0;
}