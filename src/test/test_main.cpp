#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include <iostream>

#include "../HardlightDevice.h"
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
TEST_CASE("The zone model works", "[ZoneModel]") {
	ZoneModel model;

	MyBasicHapticEvent oneshot_1(boost::uuids::random_generator()(), 0, 0.0, 1.0, 666);

	auto& pausedEvents = model.PausedEvents();
	auto& activeEvents = model.PlayingEvents();

	SECTION("There should be no playing or paused events") {
		REQUIRE(activeEvents.size() == 0);
		REQUIRE(pausedEvents.size() == 0);

		/* General testing */
		SECTION("There should be one playing event and no paused events") {
			model.Put(oneshot_1);
			REQUIRE(activeEvents.size() == 1);
			REQUIRE(pausedEvents.empty());
			REQUIRE(oneshot_1 == activeEvents.back());


			SECTION("Removing the event should result in no active and no paused events") {
				model.Remove(oneshot_1.m_id);
				REQUIRE(activeEvents.empty());
				REQUIRE(pausedEvents.empty());
			}

			SECTION("Pausing the event should result in no active and one paused event") {
				model.Pause(oneshot_1.m_id);
				REQUIRE(activeEvents.empty());
				REQUIRE(pausedEvents.size() == 1);
			}

			SECTION("Playing the event which was already added should have no effect") {
				model.Play(oneshot_1.m_id);
				REQUIRE(activeEvents.size() == 1);
				REQUIRE(pausedEvents.empty());
				REQUIRE(oneshot_1 == activeEvents.back());
			}


			SECTION("After one timestep, a PLAY command should be generated") {
				CommandBuffer result = model.Update(0.1f);
				REQUIRE(result.size() == 1);
				REQUIRE(result.back().command() == NullSpaceIPC::EffectCommand_Command_PLAY);


				SECTION("After two timesteps, no commands should be generated") {
					CommandBuffer result = model.Update(0.1f);
					REQUIRE(result.empty());
				}
			}

			SECTION("After adding a second oneshot event, there should be only one playing event (as it replaces the first) and no paused events") {
				MyBasicHapticEvent oneshot_2(boost::uuids::random_generator()(), 0, 0.0, 0.5, 666);
				model.Put(oneshot_2);
				REQUIRE(activeEvents.size() == 1);
				REQUIRE(pausedEvents.empty());
				REQUIRE(oneshot_2 == activeEvents.back());


				SECTION("After adding a continuous event, there should be only one playing event (as it replaces the first) and no paused events") {
					MyBasicHapticEvent cont_1(boost::uuids::random_generator()(), 0, 5.0, 0.3, 666);
					model.Put(cont_1);
					REQUIRE(activeEvents.size() == 1);
					REQUIRE(pausedEvents.empty());
					REQUIRE(cont_1 == activeEvents.back());


					SECTION("After one timestep, a PLAY_CONTINUOUS command should be generated") {
						CommandBuffer result = model.Update(4.9f);
						REQUIRE(result.size() == 1);
						REQUIRE(result.back().command() == NullSpaceIPC::EffectCommand_Command_PLAY_CONTINUOUS);


						SECTION("After two timesteps (past the end of the cont play), a halt should be generated") {
							CommandBuffer result = model.Update(0.1001f);
							REQUIRE(result.size() == 1);
							REQUIRE(result.back().command() == NullSpaceIPC::EffectCommand_Command_HALT);
						}
					}


					SECTION("After adding a oneshot on top of a continuous, there should be two playing events and no paused events") {
						MyBasicHapticEvent oneshot_3(boost::uuids::random_generator()(), 0, 0.0, 0.5, 666);
						model.Put(oneshot_3);
						REQUIRE(activeEvents.size() == 2);
						REQUIRE(pausedEvents.empty());
						REQUIRE(cont_1 == *(activeEvents.end() - 2));
						REQUIRE(oneshot_3 == activeEvents.back());

						
					}

					SECTION("After adding a continuous event on top of a continuous event, there should be two playing events and no paused events") {
						MyBasicHapticEvent cont_2(boost::uuids::random_generator()(), 0, 3.0, 0.3, 666);
						model.Put(cont_2);
						REQUIRE(activeEvents.size() == 2);
						REQUIRE(pausedEvents.empty());
						REQUIRE(cont_2 == activeEvents.back());
						REQUIRE(cont_1 == *(activeEvents.end() - 2));

					}
				}


			}


		}
	
		/* Cont A start -> Cont B start -> Cont B end -> Cont A resume */
		SECTION("After adding a cont play and simulating a timestep, there should be one CONT_PLAY command") {

			/*
			timestep = o

			0                      0.5                 1
			|---o----o-----|---------------|-----------|
			^first begin   ^-second begin  ^-second end ^first end
			*/
			MyBasicHapticEvent cont_1(boost::uuids::random_generator()(), 0, 1.0, 1.0, 666);
			model.Put(cont_1);
			auto commands = model.Update(0.1f);
			REQUIRE(commands.size() == 1);
			REQUIRE(commands.back().command() == NullSpaceIPC::EffectCommand_Command_PLAY_CONTINUOUS);

			SECTION("After another timestep, there should be no commands") {
				auto commands = model.Update(0.1f);
				REQUIRE(commands.size() == 0);
				

				SECTION("After adding another cont play, there should be one CONT_PLAY command") {
					MyBasicHapticEvent cont_2(boost::uuids::random_generator()(), 0, 0.3, 1.0, 666);

					model.Put(cont_2);
					auto commands = model.Update(0.1f);
					REQUIRE(commands.size() == 1);
					REQUIRE(commands.back().command() == NullSpaceIPC::EffectCommand_Command_PLAY_CONTINUOUS);


					SECTION("After another timestep, there should be no commands") {
						auto commands = model.Update(0.1f);
						REQUIRE(commands.size() == 0);

						SECTION("After a timestep past cont_2 end, there should be a cont play for cont_1") {
							auto commands = model.Update(0.11f);
							REQUIRE(commands.size() == 1);
							REQUIRE(commands.at(0).command() == NullSpaceIPC::EffectCommand_Command_PLAY_CONTINUOUS);


							SECTION("After a timestep past cont_1 end, there should be a halt") {
								auto commands = model.Update(10.0f);
								REQUIRE(commands.size() == 1);
								REQUIRE(commands.at(0).command() == NullSpaceIPC::EffectCommand_Command_HALT);

							}
						}
					}
				
				
					SECTION("After pausing cont_2, there should be one CONT_PLAY command and one paused effect") {
						model.Pause(cont_2.m_id);
						auto commands = model.Update(0.1f);
						REQUIRE(pausedEvents.size() == 1);
						REQUIRE(commands.size() == 1);
						REQUIRE(commands.back().command() == NullSpaceIPC::EffectCommand_Command_PLAY_CONTINUOUS);

						SECTION("After removing cont_2, there should be one active effect and no paused events") {
							model.Remove(cont_2.m_id);
							REQUIRE(activeEvents.size() == 1);
							REQUIRE(pausedEvents.size() == 0);
							SECTION("After time step, there should be no commands") {
								auto commands = model.Update(0.1f);
								REQUIRE(commands.size() == 0);


							}
							
						}

					}
				
				
				}
			
			
			
			}


		}
	
		
		/* Adding a ton of oneshots in the same update batch*/
		SECTION("After adding 1000 oneshots, there should be only one active effect") {
			auto gen = boost::uuids::random_generator();
			for (int i = 0; i < 1000; i++) {
				model.Put(MyBasicHapticEvent(gen(), 0, 0.0, 1.0, 666));
			}
			REQUIRE(activeEvents.size() == 1);
		}

		/* Adding a ton of cont play in the same update batch */
		SECTION("After adding 1000 cont plays, there should be 1000 active effects") {
			auto gen = boost::uuids::random_generator();
			for (int i = 0; i < 1000; i++) {
				model.Put(MyBasicHapticEvent(gen(), 0, 1.0, 1.0, 666));
			}
			REQUIRE(activeEvents.size() == 1000);
		}

		/* Adding a mix of cont play and oneshot in the same batch*/
		/* this is probably a dumb test */
		SECTION("After adding 100 oneshots, 100 cont play, then 100 oneshots, there should be 101 effects") {
			auto gen = boost::uuids::random_generator();
			for (int i = 0; i < 100; i++) {
				model.Put(MyBasicHapticEvent(gen(), 0, 0.0, 1.0, 666));
			}
			for (int i = 0; i < 100; i++) {
				model.Put(MyBasicHapticEvent(gen(), 0, 1.0, 1.0, 666));
			}
			for (int i = 0; i < 100; i++) {
				model.Put(MyBasicHapticEvent(gen(), 0, 0.0, 1.0, 666));
			}

			REQUIRE(activeEvents.back().m_duration == 0.0);
			REQUIRE((activeEvents.end() - 2)->m_duration == 1.0);
			REQUIRE((activeEvents.end() - 3)->m_duration == 1.0);

			REQUIRE(activeEvents.size() == 101);
		}

	 /* Need to halt before playing a oneshot if a continuous play was previously playing
		If you don't, the cont play gets "corrupted" and may even load up the new oneshot
	 */
		SECTION("Should not Poltergeezer") {
			MyBasicHapticEvent badboy_cont_1(boost::uuids::random_generator()(), 0, 0.1, 0.3, 1);
			MyBasicHapticEvent badboy_oneshot_1(boost::uuids::random_generator()(), 0, 0.0, 0.5, 1);

			model.Put(badboy_cont_1);
			auto commands1 = model.Update(0.05f);
			model.Put(badboy_oneshot_1);
			auto commands2 = model.Update(0.05f);
			REQUIRE(commands2.at(0).command() == NullSpaceIPC::EffectCommand_Command_HALT);
			REQUIRE(commands2.at(1).command() == NullSpaceIPC::EffectCommand_Command_PLAY);

		}
}






}



int main(int argc, char* argv[]) {
	int result = Catch::Session().run(argc, argv);

	std::cin.get();
	return (result < 0xff ? result : 0xff);

}

