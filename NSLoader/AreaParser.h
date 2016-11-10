#pragma once
#include "stdafx.h"
#include <string>
#include "Wire\IntermediateHapticFormats.h"

class AreaParser
{
public:
	AreaParser(std::string tokens);
	~AreaParser();
	AreaFlag GetArea();
private:
	AreaFlag _area;
	void parse(std::string);
};

