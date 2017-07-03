#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include <iostream>
#include "../AreaFlags.h"
#include "../Devices/HardlightDevice/HardlightDevice.h"
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include "../EventRegistry.h"
#include "../HapticsPlayer.h"
#include "../ParameterizedEvent.h"

#include <functional>
#include <chrono>

template<typename T>
T time(std::function<void()> fn) {
	auto then = std::chrono::high_resolution_clock::now();
	fn();
	auto now = std::chrono::duration_cast<T>(std::chrono::high_resolution_clock::now() - then);
	return now;
}

boost::uuids::random_generator idGenerator;
const float DELTA_TIME = 0.05f;

LiveBasicHapticEvent makeOneshot() {
	BasicHapticEventData data;
	data.area = 10;
	data.duration = 0.0;
	data.effect = 666;
	data.strength = 1.0;
	return LiveBasicHapticEvent(idGenerator(), idGenerator(), data);
}


LiveBasicHapticEvent makeCont(float duration) {
	BasicHapticEventData data;
	data.area = 10;
	data.duration = duration;
	data.effect = 555;
	data.strength = 0.5;
	return LiveBasicHapticEvent(idGenerator(), idGenerator(), data);
}
LiveBasicHapticEvent makeCont() {
	return makeCont(1.0);
}


bool isIdleCommand(const NullSpaceIPC::EffectCommand& command) {

	return command.command() == NullSpaceIPC::EffectCommand_Command_HALT;
}
bool isOneshotCommand(const NullSpaceIPC::EffectCommand& command) {
	return command.command() == NullSpaceIPC::EffectCommand_Command_PLAY;
}

bool isContCommand(const NullSpaceIPC::EffectCommand& command) {
	return command.command() == NullSpaceIPC::EffectCommand_Command_PLAY_CONTINUOUS;
}


TEST_CASE("The zone model works", "[ZoneModel]") {
	ZoneModel model(Location::Chest_Left);

	auto& pausedEvents = model.PausedEvents();
	auto& activeEvents = model.PlayingEvents();


	SECTION("A default model should have no active and no paused effects") {
		REQUIRE(activeEvents.size() == 0);
		REQUIRE(pausedEvents.size() == 0);
	}

	SECTION("Continuous play should stop and start at the correct times") {
		model.Put(makeCont(0.2f));
		auto startCommands = model.Update(DELTA_TIME);
		auto potentialStops = model.Update(DELTA_TIME * 3);
		auto stopCommands = model.Update(DELTA_TIME + .001f);
		REQUIRE(startCommands.size() == 1);
		REQUIRE(isContCommand(startCommands.at(0)));

		REQUIRE(potentialStops.empty());

		REQUIRE(stopCommands.size() == 1);
		REQUIRE(isIdleCommand(stopCommands.at(0)));
	}

	SECTION("Oneshots should generate halt, play command") {
		model.Put(makeOneshot());
		auto commands = model.Update(DELTA_TIME);
	
		REQUIRE(commands.size() == 2);
		REQUIRE(isIdleCommand(commands.at(0)));
		REQUIRE(isOneshotCommand(commands.at(1)));
	}

	SECTION("Oneshots should be removed after they generate commands") {
		model.Put(makeOneshot());
		auto commands = model.Update(DELTA_TIME);
		model.Update(DELTA_TIME);

		REQUIRE(activeEvents.empty());

	}

	SECTION("Continuous play should be removed after it generates commands") {
		model.Put(makeCont(0.1f));
		model.Update(DELTA_TIME);
		model.Update(DELTA_TIME * 2.f + .01f);
		REQUIRE(activeEvents.empty());
	}

	SECTION("A oneshot should cease generating commands after it has played") {
		model.Put(makeOneshot());
		model.Update(DELTA_TIME);
		auto c1 = model.Update(DELTA_TIME);
		auto c2 = model.Update(DELTA_TIME);
		auto c3 = model.Update(DELTA_TIME);
		REQUIRE(c1.empty());
		REQUIRE(c2.empty());
		REQUIRE(c3.empty());

	}

	SECTION("A continuous should cease generating commands after it has played") {
		model.Put(makeCont(0.1f));
		model.Update(DELTA_TIME);
		model.Update(DELTA_TIME * 2);
		auto c1 = model.Update(DELTA_TIME);
		auto c2 = model.Update(DELTA_TIME);
		auto c3 = model.Update(DELTA_TIME);

		REQUIRE(c1.empty());
		REQUIRE(c2.empty());
		REQUIRE(c3.empty());
	}

	SECTION("The model should drop all oneshots except the most recent in each batch") {
		for (int i = 0; i < 100; i++) {
			model.Put(makeOneshot());
		}
		model.Update(DELTA_TIME);
		REQUIRE(activeEvents.size() == 1);
	}
	
	SECTION("The model should not drop any continuous plays in a batch") {
		for (int i = 0; i < 100; i++) {
			model.Put(makeCont());
		}

		model.Update(DELTA_TIME);
		REQUIRE(activeEvents.size() == 100);
	}

	SECTION("Smoketests for correct dropping behavior") {
		SECTION("C -> O -> O -> C should result in C C") {
			model.Put(makeCont());
			model.Put(makeOneshot());
			model.Put(makeOneshot());
			model.Put(makeCont());

			model.Update(DELTA_TIME);
			REQUIRE(activeEvents.size() == 2);
			REQUIRE(activeEvents.at(0).isContinuous());
			REQUIRE(activeEvents.at(1).isContinuous());
		}

		SECTION("O -> O -> O -> C should result in C") {
			model.Put(makeOneshot());
			model.Put(makeOneshot());
			model.Put(makeOneshot());
			model.Put(makeCont());
			model.Update(DELTA_TIME);
			REQUIRE(activeEvents.size() == 1);
			REQUIRE(activeEvents.at(0).isContinuous());
		}

		SECTION("O -> O should result in O") {
			model.Put(makeOneshot());
			model.Put(makeOneshot());
			model.Update(DELTA_TIME);
			REQUIRE(activeEvents.size() == 1);
			REQUIRE(activeEvents.at(0).isOneshot());
		}
		SECTION("C -> C -> O -> O should result in C C O") {
			model.Put(makeCont());
			model.Put(makeCont());
			model.Put(makeOneshot());
			model.Put(makeOneshot());
			model.Update(DELTA_TIME);
			REQUIRE(activeEvents.size() == 3);
			REQUIRE(activeEvents.at(0).isContinuous());
			REQUIRE(activeEvents.at(1).isContinuous());
			REQUIRE(activeEvents.at(2).isOneshot());

		}
	}
	


	//Bug: Poltergeezer
	//If you play a oneshot while a cont play is happening, you must send a halt first
	//If you don'tm the cont play gets "corrupted" and may even load up with the new oneshot
	//thus playing the motor incorrectly.
	SECTION("Should not Poltergeezer") {
		model.Put(makeCont());
		model.Update(DELTA_TIME);
		model.Put(makeOneshot());
		auto commands = model.Update(DELTA_TIME);
		REQUIRE(commands.size() == 2);
		REQUIRE(isIdleCommand(commands.at(0)));
		REQUIRE(isOneshotCommand(commands.at(1)));
	}
	
	SECTION("Effects should be layered based upon recency (newest takes priority)") {

		SECTION("A newer cont play should play over an older one") {

			auto bottomLayer = makeCont();
			auto topLayer = makeCont();

			model.Put(bottomLayer);
			model.Update(DELTA_TIME);
		    model.Put(topLayer);
			auto commands = model.Update(DELTA_TIME);

			REQUIRE(commands.size() == 1);
			REQUIRE(isContCommand(commands.at(0)));

		}

		SECTION("A short-running cont should yield to a longer running cont when it is finished") {
			auto bottomLayer = makeCont(1.0f);
			auto topLayer = makeCont(0.1f);

			model.Put(bottomLayer);
			model.Update(DELTA_TIME);
			model.Put(topLayer);

			auto ignoreTopPlayCommands = model.Update(DELTA_TIME);
			auto commands = model.Update(DELTA_TIME * 2.f + .01f);

			REQUIRE(commands.size() == 1);
			REQUIRE(isContCommand(commands.at(0)));
		}
	}


	SECTION("Playback commands should work") {
		SECTION("Pausing a nonexistant effect should not cause any exceptions") {
			REQUIRE_NOTHROW(model.Pause(boost::uuids::uuid()));
		}
		SECTION("Playing a nonexistant effect should not cause any exceptions") {
			REQUIRE_NOTHROW(model.Play(boost::uuids::uuid()));
		}
		SECTION("Removing a nonexistant effect should not cause any exceptions") {
			REQUIRE_NOTHROW(model.Remove(boost::uuids::uuid()));
		}


		SECTION("Pausing and playing cont play should move it to the correct lists") {
			auto cont = makeCont();
			model.Put(cont);
			model.Update(DELTA_TIME);
			model.Pause(cont.GetParentId());
			model.Update(DELTA_TIME);
			REQUIRE(activeEvents.empty());
			REQUIRE(pausedEvents.size() == 1);

			model.Play(cont.GetParentId());
			model.Update(DELTA_TIME);
			REQUIRE(pausedEvents.empty());
			REQUIRE(activeEvents.size() == 1);
		}

		SECTION("Pausing and resuming play should generate the correct commands") {
			auto cont = makeCont();
			model.Put(cont);
			model.Update(DELTA_TIME);
			model.Pause(cont.GetParentId());
			auto commands = model.Update(DELTA_TIME);

			REQUIRE(commands.size() == 1);
			REQUIRE(isIdleCommand(commands.at(0)));

			model.Play(cont.GetParentId());
			commands = model.Update(DELTA_TIME);
			REQUIRE(commands.size() == 1);
			REQUIRE(isContCommand(commands.at(0)));
		}
		
		
		SECTION("It should (?) be possible to command an effect in the same batch that it is sent") {
			auto cont = makeCont();
			model.Put(cont);
			model.Pause(cont.GetParentId());
			model.Update(DELTA_TIME);
			REQUIRE(pausedEvents.size() == 1);
		}
	}
}










TEST_CASE("The motor state changer works", "[MotorStateChanger]") {
	using MotorFirmwareState = MotorStateChanger::MotorFirmwareState;

	auto trans = MotorStateChanger(Location::Chest_Left);

	REQUIRE(MotorFirmwareState::Idle == trans.GetState());

	SECTION("Transitions should produce the correct states") {

		SECTION("Adding a oneshot to an idle motor should transition to playing_oneshot") {
			trans.transitionTo(makeOneshot());
			REQUIRE(MotorFirmwareState::PlayingOneshot == trans.GetState());
		}

		SECTION("Adding a cont to an idle motor should transition to playing_cont") {
			trans.transitionTo(makeCont());
			REQUIRE(MotorFirmwareState::PlayingContinuous == trans.GetState());
		}

		SECTION("Halting from oneshot should transition to idle") {
			trans.transitionTo(makeOneshot());
			trans.transitionToIdle();
			REQUIRE(MotorFirmwareState::Idle == trans.GetState());
		}

		SECTION("Halting from cont should transition to idle ") {
			trans.transitionTo(makeCont());
			trans.transitionToIdle();
			REQUIRE(MotorFirmwareState::Idle == trans.GetState());
		}

		SECTION("Halting from idle should transition to idle") {
			trans.transitionToIdle();
			REQUIRE(MotorFirmwareState::Idle == trans.GetState());
		}

		SECTION("Transitioning to cont from oneshot should result in PlayingContinuous") {
			trans.transitionTo(makeOneshot());
			trans.transitionTo(makeCont());
			REQUIRE(MotorFirmwareState::PlayingContinuous == trans.GetState());
		}

		SECTION("Transitioning to oneshot from cont should result in PlayingOneshot") {
			trans.transitionTo(makeCont());
			trans.transitionTo(makeOneshot());
			REQUIRE(MotorFirmwareState::PlayingOneshot == trans.GetState());
		}


	}

	SECTION("Transitioning should produce the correct commands") {
		SECTION("Adding a oneshot to an idle motor should produce HALT, PLAY command") {
			auto commands = trans.transitionTo(makeOneshot());
			REQUIRE(commands.size() == 2);
			REQUIRE(isIdleCommand(commands.at(0)));
			REQUIRE(isOneshotCommand(commands.at(1)));

		}

		SECTION("Adding a oneshot on top of a continuous should produce HALT followed by PLAY commands") {
			trans.transitionTo(makeCont());
			auto commands = trans.transitionTo(makeOneshot());
			REQUIRE(commands.size() == 2);
			REQUIRE(isIdleCommand(commands.at(0)));
			REQUIRE(isOneshotCommand(commands.at(1)));
		}


		SECTION("Adding a oneshot on top of a oneshot should produce HALT, PLAY command") {
			trans.transitionTo(makeOneshot());
			auto commands = trans.transitionTo(makeOneshot());
			REQUIRE(commands.size() == 2);
			REQUIRE(isIdleCommand(commands.at(0)));
			REQUIRE(isOneshotCommand(commands.at(1)));
		}

		SECTION("Adding a continuous to an idle motor should produce PLAY_CONT command") {
			auto commands = trans.transitionTo(makeCont());
			REQUIRE(commands.size() == 1);
			REQUIRE(isContCommand(commands.at(0)));
		}

		SECTION("Adding a continuous on top of a oneshot should produce PLAY_CONT command") {
			trans.transitionTo(makeOneshot());
			auto commands = trans.transitionTo(makeCont());
			REQUIRE(commands.size() == 1);
			REQUIRE(isContCommand(commands.at(0)));
		}

		SECTION("Adding a continuous on top of a continuous should produce  PLAY_CONT command") {
			trans.transitionTo(makeCont());
			auto commands = trans.transitionTo(makeCont());
			REQUIRE(commands.size() == 1);
			REQUIRE(isContCommand(commands.at(0)));
		}




	}


	SECTION("Halting should produce the correct commands") {
		SECTION("Halting from idle should result in no commands") {
			auto commands = trans.transitionToIdle();
			REQUIRE(commands.empty());
		}

		SECTION("Halting from a oneshot should result in no commands") {
			trans.transitionTo(makeOneshot());
			auto commands = trans.transitionToIdle();
			REQUIRE(commands.empty());
		}

		SECTION("Halting from a continuous should result in a HALT command") {
			trans.transitionTo(makeCont());
			auto commands = trans.transitionToIdle();
			REQUIRE(commands.size() == 1);
			REQUIRE(isIdleCommand(commands.at(0)));
		}
	}

}


std::vector<std::unique_ptr<PlayableEvent>> makePlayables() {
	std::vector<std::unique_ptr<PlayableEvent>> events;
	BasicHapticEvent a;
	ParameterizedEvent e(NSVR_EventType_BasicHapticEvent);
	e.SetInt("area", (int)AreaFlag::Chest_Both);
	e.SetFloat("time", 0.0f);
	
	a.parse(e);
	events.push_back(std::unique_ptr<PlayableEvent>(new BasicHapticEvent(a)));
	e.SetFloat("time", 1.0f);
	a.parse(e);
	events.push_back(std::unique_ptr<PlayableEvent>(new BasicHapticEvent(a)));
	return events;
}
TEST_CASE("The haptics player works", "[HapticsPlayer]") {
	EventRegistry registry;
	HapticsPlayer player(registry);

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
		REQUIRE(info->State() != NSVR_EffectInfo_State_Playing);
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
		REQUIRE(info->State() != NSVR_EffectInfo_State_Playing);
		REQUIRE(info->CurrentTime() == Approx(0.0f));
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
		REQUIRE(info->State() == NSVR_EffectInfo_State_Playing);
		REQUIRE(info->CurrentTime() == Approx(DELTA_TIME * 2));
	}

	SECTION("An effect should stop after reaching its duration") {
		HapticHandle h = player.Create(makePlayables());

		auto info = player.GetHandleInfo(h);
		REQUIRE(info->Duration() > DELTA_TIME);


		player.Play(h);
		player.Update(info->Duration() + DELTA_TIME);
		info = player.GetHandleInfo(h);
		REQUIRE(info->State() != NSVR_EffectInfo_State_Playing);
		REQUIRE(info->CurrentTime() == Approx(0.0f));
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

	SECTION("When you play an effect, it should .. work") {
		HardlightDevice device;
		device.RegisterDrivers(registry);
		
		HapticHandle h = player.Create(makePlayables());
		player.Play(h);
		player.Update(DELTA_TIME);
		auto cmds = device.GenerateHardwareCommands(DELTA_TIME);
		REQUIRE(cmds.size() > 0);
		REQUIRE(player.GetNumLiveEffects() == 1);

		DisplayResults effects = device.QueryDrivers();
		REQUIRE(!effects.empty());
		player.Update(DELTA_TIME);
		 cmds = device.GenerateHardwareCommands(DELTA_TIME);
		 REQUIRE(cmds.empty());

		 player.Update(DELTA_TIME*20);
		 cmds = device.GenerateHardwareCommands(DELTA_TIME*20);
		 REQUIRE(cmds.size() > 0);

		 player.Update(DELTA_TIME * 20);
		 cmds = device.GenerateHardwareCommands(DELTA_TIME * 20);
		 REQUIRE(cmds.empty());
		 

	}
	

	


	
	
		
	

}


TEST_CASE("The events system works", "[EventSystem]") {

	ParameterizedEvent event(NSVR_EventType_BasicHapticEvent);

	SECTION("You should be able to get out what you put in") {
		event.Set("key1", 1.0f);
		REQUIRE(event.Get("key1", 999.0f) == Approx(1.0f));

		event.Set("key2", 1);
		REQUIRE(event.Get("key2", 999) == 1);

		event.Set("key3", std::vector<int>({ 1 }));
		REQUIRE(event.Get("key3", std::vector<int>({ 999 })).at(0) == 1);
	}

	SECTION("You should get a default value if you supply a wrong key") {
		event.Set("key1", 1.0f);
		REQUIRE(event.Get("key2", 999) == 999);
	}

	SECTION("You should get a default value if you supply the wrong type") {
		event.Set("key1", 1.0f);
		REQUIRE(event.Get("key1", 999) == 999);
	}

	SECTION("If you overwrite a key with a different value, you should get the new value out") {
		event.Set("key1", 1.0f);
		event.Set("key1", 2.0f);
		REQUIRE(event.Get("key1", 999.0f) == Approx(2.0f));
	}

	SECTION("If you overwrite a key with a different type, you should get the new type out") {
		event.Set("key1", 1.0f);
		event.Set("key1", 2);
		REQUIRE(event.Get("key1", 999) == 2);
	}

	SECTION("If you request a key that isn't present, you should get the default value") {
		REQUIRE(event.Get("key1", 999) == 999);
		REQUIRE(event.Get("key2", 999.0f) == 999.0f);
		REQUIRE(event.Get("key3", std::vector<float>({ 999.0f })).at(0) == Approx(999.0f));
		REQUIRE(event.Get("key4", std::vector<int>({ 999 })).at(0) == 999);
	}
}


int main(int argc, char* argv[]) {
	int result = Catch::Session().run(argc, argv);

	std::cin.get();
	return (result < 0xff ? result : 0xff);

}

