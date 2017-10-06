#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include <iostream>
#include "../AreaFlags.h"
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include "../HapticsPlayer.h"
#include "../ParameterizedEvent.h"
#include <boost/asio/io_service.hpp>
#include "../ClientMessenger.h"
#include <functional>
#include <chrono>
#include "../SharedCommunication/readablesharedvector.h"
#include "../SharedCommunication/SharedTypes.h"
#include "HLVR_Experimental.h"
template<typename T>
T time(std::function<void()> fn) {
	auto then = std::chrono::high_resolution_clock::now();
	fn();
	auto now = std::chrono::duration_cast<T>(std::chrono::high_resolution_clock::now() - then);
	return now;
}

boost::uuids::random_generator idGenerator;
const float DELTA_TIME = 0.05f;



bool isIdleCommand(const NullSpaceIPC::EffectCommand& command) {

	return command.command() == NullSpaceIPC::EffectCommand_Command_HALT;
}
bool isOneshotCommand(const NullSpaceIPC::EffectCommand& command) {
	return command.command() == NullSpaceIPC::EffectCommand_Command_PLAY;
}

bool isContCommand(const NullSpaceIPC::EffectCommand& command) {
	return command.command() == NullSpaceIPC::EffectCommand_Command_PLAY_CONTINUOUS;
}






std::vector<std::unique_ptr<PlayableEvent>> makePlayables() {
	std::vector<std::unique_ptr<PlayableEvent>> events;
	BasicHapticEvent a;
	ParameterizedEvent e(HLVR_EventType_SimpleHaptic);
	std::vector<uint32_t> region = { hlvr_region_upper_ab_left };
	e.Set(HLVR_EventKey_SimpleHaptic_Regions_UInt32s, region.data(), 1);
	e.Set(HLVR_EventKey_Time_Float, 0.0f);
	
	a.parse(e);
	events.push_back(std::unique_ptr<PlayableEvent>(new BasicHapticEvent(a)));
	e.Set(HLVR_EventKey_Time_Float, 1.0f);
	a.parse(e);
	events.push_back(std::unique_ptr<PlayableEvent>(new BasicHapticEvent(a)));
	return events;
}
TEST_CASE("The haptics player works", "[HapticsPlayer]") {

	//Now I see why we shouldn't have classes take hard references to resources.
	//Messenger should probably be an interface, else we need it to test HapticsPlayer
	boost::asio::io_service io;
	ClientMessenger m(io);
	HapticsPlayer player( m);

	REQUIRE(player.GetNumLiveEffects() == 0);
	REQUIRE(player.GetNumReleasedEffects() == 0);


	SECTION("Retrieving a nonexistent handle shouldn't crash") {
		HapticHandle randomlyChosen = 1245;
		REQUIRE_NOTHROW([&]() {
			auto handle = player.GetHandleInfo(randomlyChosen);
			REQUIRE(!handle);
		});
		
	}

	SECTION("Creating an effect should work, and the effect should be not be playing by default") {
		HapticHandle h = player.Create(makePlayables());
		
		REQUIRE(player.GetNumLiveEffects() == 1);

		auto info = player.GetHandleInfo(h);
		REQUIRE(info->State() != 0); //this enum should be exposed. 0 = playing
		REQUIRE(info->CurrentTime() == Approx(0.0f));
	}

	SECTION("Pausing an effect should work") {
		HapticHandle h = player.Create(makePlayables());
		
		auto info = player.GetHandleInfo(h);
		REQUIRE(info->Duration() > DELTA_TIME);

		player.Play(h);
		player.Update(DELTA_TIME);
		player.Pause(h);

		info = player.GetHandleInfo(h);
		REQUIRE(info->State() != HLVR_EffectInfo_State_Playing);
		REQUIRE(info->CurrentTime() == Approx(DELTA_TIME));
	}

	SECTION("Stopping an effect should work") {
		HapticHandle h = player.Create(makePlayables());
		auto info = player.GetHandleInfo(h);
		REQUIRE(info->Duration() > DELTA_TIME);

		player.Play(h);
		player.Update(DELTA_TIME);
		player.Stop(h);

		info = player.GetHandleInfo(h);
		REQUIRE(info->State() != HLVR_EffectInfo_State_Playing);
		REQUIRE(info->CurrentTime() == Approx(DELTA_TIME));
	}

	SECTION("Resuming an effect should work") {
		HapticHandle h = player.Create(makePlayables());
		auto info = player.GetHandleInfo(h);
		REQUIRE(info->Duration() > DELTA_TIME);

		player.Play(h);
		player.Update(DELTA_TIME);
		player.Pause(h);
		player.Update(DELTA_TIME * 10);
		player.Play(h);
		player.Update(DELTA_TIME);

		info = player.GetHandleInfo(h);
		REQUIRE(info->State() == HLVR_EffectInfo_State_Playing);
		REQUIRE(info->CurrentTime() == Approx(DELTA_TIME * 2));
	}

	SECTION("An effect should stop after reaching its duration") {
		HapticHandle h = player.Create(makePlayables());

		auto info = player.GetHandleInfo(h);
		REQUIRE(info->Duration() > DELTA_TIME);


		player.Play(h);
		player.Update(info->Duration() + DELTA_TIME);
		info = player.GetHandleInfo(h);
		REQUIRE(info->State() != HLVR_EffectInfo_State_Playing);
		REQUIRE(info->CurrentTime() == Approx(info->Duration() + DELTA_TIME));
	}

	SECTION("Releasing an effect should work") {
		HapticHandle h = player.Create(makePlayables());
		player.Release(h);
		REQUIRE(player.GetNumLiveEffects() == 0);
		REQUIRE(player.GetNumReleasedEffects() == 1);
		
	}

	SECTION("A released effect should be cleaned up properly") {
		HapticHandle h = player.Create(makePlayables());

		SECTION("If it was playing at the time of release, it should not be deleted until it is done playing") {
			player.Play(h);
			auto duration = player.GetHandleInfo(h)->Duration();
			player.Release(h);
			player.Update(DELTA_TIME);
			REQUIRE(player.GetNumReleasedEffects() == 1);
			player.Update(DELTA_TIME);
			REQUIRE(player.GetNumReleasedEffects() == 1);

			player.Update(duration + DELTA_TIME);
			REQUIRE(player.GetNumLiveEffects() == 0);
			REQUIRE(player.GetNumReleasedEffects() == 0);
		}

		SECTION("If it was not playing at time of release, it should be deleted in the next update") {
			player.Release(h);
			player.Update(DELTA_TIME);
			REQUIRE(player.GetNumReleasedEffects() == 0);
			REQUIRE(player.GetNumLiveEffects() == 0);
		}


		

	}


	


	
	
		
	

}


TEST_CASE("The events system works", "[EventSystem]") {

	ParameterizedEvent event(HLVR_EventType_SimpleHaptic);

	SECTION("You should be able to get out what you put in") {
		event.Set(HLVR_EventKey_SimpleHaptic_Duration_Float, 1.0f);
		REQUIRE(event.GetOr(HLVR_EventKey_SimpleHaptic_Duration_Float, 999.0f) == Approx(1.0f));

		event.Set(HLVR_EventKey_SimpleHaptic_Effect_Int, 1);
		REQUIRE(event.GetOr(HLVR_EventKey_SimpleHaptic_Effect_Int, 999) == 1);

		event.Set(static_cast<HLVR_EventKey>(10001), std::vector<int>({ 1 }));
		REQUIRE(event.GetOr(static_cast<HLVR_EventKey>(10001), std::vector<int>({ 999 })).at(0) == 1);
	}

	SECTION("You should get a default value if you supply a wrong key") {
		event.Set(HLVR_EventKey_SimpleHaptic_Duration_Float, 1.0f);
		REQUIRE(event.GetOr(HLVR_EventKey_SimpleHaptic_Effect_Int, 999) == 999);
	}

	SECTION("You should get a default value if you supply the wrong type") {
		event.Set(HLVR_EventKey_SimpleHaptic_Duration_Float, 1.0f);
		REQUIRE(event.GetOr(HLVR_EventKey_SimpleHaptic_Duration_Float, 999) == 999);
	}

	SECTION("If you overwrite a key with a different value, you should get the new value out") {
		event.Set(HLVR_EventKey_SimpleHaptic_Duration_Float, 1.0f);
		event.Set(HLVR_EventKey_SimpleHaptic_Duration_Float, 2.0f);
		REQUIRE(event.GetOr(HLVR_EventKey_SimpleHaptic_Duration_Float, 999.0f) == Approx(2.0f));
	}

	SECTION("If you overwrite a key with a different type, you should get the new type out") {
		event.Set(HLVR_EventKey_SimpleHaptic_Duration_Float, 1.0f);
		event.Set(HLVR_EventKey_SimpleHaptic_Duration_Float, 2);
		REQUIRE(event.GetOr(HLVR_EventKey_SimpleHaptic_Duration_Float, 999) == 2);
	}

	SECTION("If you request a key that isn't present, you should get the default value") {
		REQUIRE(event.GetOr(HLVR_EventKey_SimpleHaptic_Duration_Float, 999) == 999);
		REQUIRE(event.GetOr(HLVR_EventKey_SimpleHaptic_Effect_Int, 999.0f) == 999.0f);
		REQUIRE(event.GetOr(HLVR_EventKey_SimpleHaptic_Strength_Float, std::vector<float>({ 999.0f })).at(0) == Approx(999.0f));
		REQUIRE(event.GetOr(HLVR_EventKey_SimpleHaptic_Regions_UInt32s, std::vector<int>({ 999 })).at(0) == 999);
	}

	SECTION("TryGet will return false if the key is not found") {
		int x;
		REQUIRE(!event.TryGet(HLVR_EventKey_SimpleHaptic_Effect_Int, &x));
	}

	SECTION("TryGet will return default value if the key is not found") {
		int x = 152;
		event.TryGet(HLVR_EventKey_SimpleHaptic_Effect_Int, &x);
		REQUIRE(x == 0);
	}

	SECTION("TryGet will return the value if found") {
		int x;
		event.Set(HLVR_EventKey_SimpleHaptic_Effect_Int, 152);
		REQUIRE(event.TryGet(HLVR_EventKey_SimpleHaptic_Effect_Int, &x));
		REQUIRE(x == 152);
	}
}

TEST_CASE("Retrieving the service version should work") {
	boost::asio::io_service io;

	ClientMessenger m(io);
	io.run_one();
	io.run_one();
	io.run_one();
	io.run_one();
	io.run_one();

	//This is a weird case setup,  I just want to see if it works
	SECTION("So does it?") {
		HLVR_PlatformInfo info = { 0 };
		auto v = m.ConnectedToService(&info);
		if (v) {
			if (info.MajorVersion == 0) {
				REQUIRE(info.MinorVersion > 0);
			}
			else {
				REQUIRE(info.MinorVersion >= 0);
				REQUIRE(info.MajorVersion >= 1);
			}
		}
		
	}
}
TEST_CASE("BodyView should work") {
	boost::asio::io_service io;
	
	ClientMessenger m(io);
	io.run_one();

	SECTION("The shared memory should instantiate") {
		auto r = m.ReadBodyView();
	}
}
int main(int argc, char* argv[]) {
	int result = Catch::Session().run(argc, argv);

	std::cin.get();
	return (result < 0xff ? result : 0xff);

}

