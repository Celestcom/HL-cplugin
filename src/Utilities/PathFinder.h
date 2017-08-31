#pragma once

#include <unordered_map>
#include "SharedTypes.h"
class PathFinder {
public:
	using named_region = NullSpace::SharedMemory::nsvr_shared_region;

	std::vector<named_region> ShortestPath(named_region from, named_region to) const;
	std::vector<std::vector<named_region>> Emanation(named_region from, unsigned int depth);
	PathFinder();
private:
	std::unordered_map<named_region, std::vector<named_region>> m_edges;
	std::unordered_map<named_region, std::unordered_map<named_region, int>> m_weights;
	const static std::vector<named_region> m_regions;
	void insertBidirectionalEdge(named_region from, named_region to, int weight);
	int cost(named_region from, named_region to);
};