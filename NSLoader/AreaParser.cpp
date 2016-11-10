#include "stdafx.h"
#include "AreaParser.h"
#include "Locator.h"
#include <boost/algorithm/string.hpp>
AreaParser::AreaParser(std::string tokens):_area(AreaFlag::None)
{
	parse(tokens);
}


AreaParser::~AreaParser()
{
}

AreaFlag AreaParser::GetArea()
{
	return _area;
}

void AreaParser::parse(std::string areaString)
{
	auto& translator = Locator::getTranslator();
	std::vector<std::string> tokens;
	boost::split(tokens, areaString, boost::is_any_of("|"));
	AreaFlag area = AreaFlag::None;
	for (auto possibleArea : tokens) {
		area |= translator.ToArea(possibleArea, AreaFlag::None);
	}
	_area = area;

}
