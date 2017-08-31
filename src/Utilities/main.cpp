#include "stdafx.h"
#include "PathFinder.h"
#include "utilities.h"
int main() {

	NSVR_System* system;
	NSVR_System_Create(&system);


	NSVR_Util_StrikeParams params = { 0 };
	params.Effect = NSVR_Effect_Click;
	params.FromRegion = static_cast<uint32_t>(PathFinder::named_region::identifier_lower_ab_left);
	params.ToRegion = static_cast<uint32_t>(PathFinder::named_region::identifier_chest_right);
	params.TotalDuration = 4.0;

	NSVR_Timeline* timeline;
	NSVR_Timeline_Create(&timeline);
	NSVR_Util_Strike(timeline, &params);


	NSVR_PlaybackHandle* handle;
	NSVR_PlaybackHandle_Create(&handle);

	NSVR_Timeline_Transmit(timeline, system, handle);

	NSVR_Timeline_Release(&timeline);




	std::cin.get();
	NSVR_PlaybackHandle_Command(handle, NSVR_PlaybackCommand_Play);
	std::cin.get();
	return 0;
}