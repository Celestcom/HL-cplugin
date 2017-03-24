#pragma once
#include "StdAfx.h"
#include "Enums.h"
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include "Wire\IntermediateHapticFormats.h"
using namespace boost::bimaps;

typedef bimap<Effect, std::string> EffectMap;
typedef bimap<Location, std::string> LocationMap;
typedef bimap<Side, std::string> SideMap;
typedef bimap<JsonLocation, std::string> JsonLocationMap;
typedef bimap<AreaFlag, std::string> AreaMap;
typedef bimap<std::string, uint8_t> EffectFamilyMap;

class EnumTranslator
{
public:
	EnumTranslator();
	~EnumTranslator();

	std::string ToString(Location loc) const;
	Location ToLocation(std::string location) const;
	Location ToLocation(std::string location, Location defaultLocation);


	std::string ToString(Effect effect) const;
	Effect ToEffect(std::string effect) const;
	Effect ToEffect(std::string effect, Effect defaultEffect);
	
	std::string ToString(Side side) const;
	Side ToSide(std::string side) const;
	Side ToSide(std::string side, Side defaultSide);

	std::string ToString(JsonLocation jsonLoc) const;
	JsonLocation ToJsonLocation(std::string jsonLoc) const;
	JsonLocation ToJsonLocation(std::string jsonLoc, JsonLocation defaultLocation);
	AreaFlag ToArea(std::string area, AreaFlag default) const;

	AreaFlag ToArea(std::string area) const;
	std::string ToString(AreaFlag area) const;

	uint32_t ToEffectFamily(std::string effectFamily) const;
	std::string ToString(uint32_t effectFamily) const;
private:
	
	void init_locations();
	void init_effects();
	void init_sides();
	void init_json_locations();
	void init_areas();
	void init_familymap();

	AreaMap _areaMap;
	EffectMap _effectMap;
	LocationMap _locationMap;
	SideMap _sideMap;
	JsonLocationMap _jsonLocationMap;
	EffectFamilyMap _effectFamilyMap;
};

