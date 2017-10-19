#pragma once
#include <cstdint>
#include <vector>
#include <boost/variant.hpp>

struct TargetRegions {
	std::vector<uint32_t> regions;
};

inline bool operator==(const TargetRegions& lhs, const TargetRegions& rhs) {
	return lhs.regions == rhs.regions;
}

struct TargetNodes {
	std::vector<uint32_t> nodes;
};

inline bool operator==(const TargetNodes& lhs, const TargetNodes& rhs) {
	return lhs.nodes == rhs.nodes;
}

using Target = boost::variant<TargetRegions, TargetNodes>;
