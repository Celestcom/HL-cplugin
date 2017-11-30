#pragma once
#include "stdafx.h"
#include "Enums.h"
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
using namespace boost::bimaps;

typedef bimap<Effect, std::string> EffectMap;
typedef bimap<Location, std::string> LocationMap;
typedef bimap<std::string, uint32_t> EffectFamilyMap;


class EnumTranslator
{
public:
	EnumTranslator();

	std::string ToString(Location loc) const;
	Location ToLocation(std::string location) const;
	Location ToLocation(std::string location, Location defaultLocation);


	

	uint32_t ToEffectFamily(std::string effectFamily) const;
	std::string ToEffectFamilyString(uint32_t effectFamily) const;

	
private:
	
	void init_locations();
	void init_effects();
	void init_familymap();

	EffectMap _effectMap;
	LocationMap _locationMap;
	EffectFamilyMap _effectFamilyMap;
};

