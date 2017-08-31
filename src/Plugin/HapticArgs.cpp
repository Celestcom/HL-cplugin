#include "stdafx.h"
#include "HapticArgs.h"


#include "Enums.h"



PatternArgs::PatternArgs(const std::string& name, ::Side side): Side(side), Name(name)
{
	
}

PatternArgs::~PatternArgs()
{
}

std::size_t PatternArgs::GetCombinedHash() const 
{
	std::size_t s = 17;
    hash_combine(s, this->Name, this->Side);
	return s;
}

SequenceArgs::SequenceArgs(const std::string& name, AreaFlag loc, float strength) : Location(loc), Name(name), Strength(strength)
{
}

SequenceArgs::~SequenceArgs()
{
}

std::size_t SequenceArgs::GetCombinedHash() const 
{
	std::size_t s = 17;
	hash_combine(s, this->Name, this->Location, this->Strength);
	return s;
}

ExperienceArgs::ExperienceArgs(const std::string& name, ::Side side) : Side(side), Name(name)
{
}

ExperienceArgs::~ExperienceArgs()
{
}

std::size_t ExperienceArgs::GetCombinedHash() const 
{
	std::size_t s = 17;
	hash_combine(s, this->Name, this->Side);
	return s;
}
