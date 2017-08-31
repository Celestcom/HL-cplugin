#include "stdafx.h"
#include "PathFinder.h"
#include <queue>
PathFinder::named_region findClosest(
	const std::vector<PathFinder::named_region>& neighbors, 
	const std::unordered_map<PathFinder::named_region, 
	float>& distances) 

{
	PathFinder::named_region result = *std::min_element(neighbors.begin(), neighbors.end(), [&distances](const auto& a, const auto& b) {
		return distances.at(a) < distances.at(b);
	});

	return result;
}

const std::vector<PathFinder::named_region> PathFinder::m_regions = {
	PathFinder::named_region::identifier_unknown,
	PathFinder::named_region::identifier_body,
	PathFinder::named_region::identifier_torso,
	PathFinder::named_region::identifier_torso_front,
	PathFinder::named_region::identifier_chest_left,
	PathFinder::named_region::identifier_chest_right,
	PathFinder::named_region::identifier_upper_ab_left,
	PathFinder::named_region::identifier_middle_ab_left,
	PathFinder::named_region::identifier_lower_ab_left,
	PathFinder::named_region::identifier_upper_ab_right,
	PathFinder::named_region::identifier_middle_ab_right ,
	PathFinder::named_region::identifier_lower_ab_right ,
	PathFinder::named_region::identifier_torso_back ,
	PathFinder::named_region::identifier_torso_left ,
	PathFinder::named_region::identifier_torso_right ,
	PathFinder::named_region::identifier_upper_back_left ,
	PathFinder::named_region::identifier_upper_back_right ,
	PathFinder::named_region::identifier_upper_arm_left ,
	PathFinder::named_region::identifier_lower_arm_left ,
	PathFinder::named_region::identifier_upper_arm_right ,
	PathFinder::named_region::identifier_lower_arm_right ,
	PathFinder::named_region::identifier_shoulder_left ,
	PathFinder::named_region::identifier_shoulder_right ,
	PathFinder::named_region::identifier_upper_leg_left ,
	PathFinder::named_region::identifier_lower_leg_left ,
	PathFinder::named_region::identifier_upper_leg_right ,
	PathFinder::named_region::identifier_lower_leg_right ,
	PathFinder::named_region::identifier_head ,
	PathFinder::named_region::identifier_palm_left ,
	PathFinder::named_region::identifier_palm_right
};

std::vector<PathFinder::named_region> PathFinder::ShortestPath(named_region from, named_region to) const
{
	std::unordered_map<named_region, float> distances;
	std::unordered_map<named_region, named_region> previous;
	std::vector<named_region> toVisit;
	toVisit.reserve(m_regions.size());

	for (named_region area : m_regions) {
		distances[area] = std::numeric_limits<float>::max();
		previous[area] = named_region::identifier_unknown;
		toVisit.push_back(area);
	}


	distances[from] = 0;

	while (!toVisit.empty()) {
		named_region closest = findClosest(toVisit, distances);
		//std::cout << "Closest found: " << int(closest) << '\n';
		if (closest == to) {
			break;
		}
		toVisit.erase(std::remove(toVisit.begin(), toVisit.end(), closest), toVisit.end());

		const auto& neighbors = m_edges.at(closest);
		for (named_region neighbor : neighbors) {

			float alt = distances[closest] + m_weights.at(closest).at(neighbor);
			if (alt < distances[neighbor]) {
				distances[neighbor] = alt;
				previous[neighbor] = closest;
			}
		}
	}

	std::vector<named_region> result;
	named_region prev = to;
	while (previous[prev] != named_region::identifier_unknown) {
		result.push_back(prev);
		prev = previous[prev];
	}

	result.push_back(from);

	std::reverse(result.begin(), result.end());
	return result;

}

std::vector<std::vector<PathFinder::named_region>> PathFinder::Emanation(named_region from, unsigned int depth)
{
	std::vector<std::vector<named_region>> stages;

	std::unordered_map<named_region, bool> visited;
	std::queue<named_region> toVisit;

	unsigned int currentDepth = 0;
	unsigned int elementsToDepthIncrease = 1;
	unsigned int nextElementsToDepthIncrease = 0;

	visited[from] = true;
	toVisit.push(from);

	stages.push_back(std::vector<named_region>{from});

	std::vector<named_region> potentialNextStage;

	while (!toVisit.empty()) {
		named_region current = toVisit.front();
		toVisit.pop();

		const auto& neighbors = m_edges.at(current);
		for (named_region neighbor : neighbors) {
			if (!visited[neighbor]) {
				visited[neighbor] = true;
				toVisit.push(neighbor);
				potentialNextStage.push_back(neighbor);
				nextElementsToDepthIncrease += 1;
			}
		}

		elementsToDepthIncrease -= 1;
		if (elementsToDepthIncrease == 0) {
			if (!potentialNextStage.empty()) { stages.push_back(potentialNextStage); }
			currentDepth += 1;
			if (currentDepth == depth) { return stages; }
			elementsToDepthIncrease = nextElementsToDepthIncrease;
			nextElementsToDepthIncrease = 0;
			potentialNextStage.clear();
		}

	}

	return stages;

}

PathFinder::PathFinder()
{
	insertBidirectionalEdge(named_region::identifier_chest_left, named_region::identifier_chest_right, 10);
	insertBidirectionalEdge(named_region::identifier_chest_left, named_region::identifier_upper_ab_left, 10);
	insertBidirectionalEdge(named_region::identifier_chest_left, named_region::identifier_upper_ab_right, 17);
	insertBidirectionalEdge(named_region::identifier_chest_left, named_region::identifier_shoulder_left, 10);

	//Right chest
	insertBidirectionalEdge(named_region::identifier_chest_right, named_region::identifier_chest_left, 10);
	insertBidirectionalEdge(named_region::identifier_chest_right, named_region::identifier_upper_ab_right, 10);
	insertBidirectionalEdge(named_region::identifier_chest_right, named_region::identifier_upper_ab_left, 17);
	insertBidirectionalEdge(named_region::identifier_chest_right, named_region::identifier_shoulder_right, 10);

	//Left Upper_ Ab
	insertBidirectionalEdge(named_region::identifier_upper_ab_left, named_region::identifier_chest_left, 10);
	insertBidirectionalEdge(named_region::identifier_upper_ab_left, named_region::identifier_middle_ab_left, 10);
	insertBidirectionalEdge(named_region::identifier_upper_ab_left, named_region::identifier_upper_ab_right, 10);
	insertBidirectionalEdge(named_region::identifier_upper_ab_left, named_region::identifier_middle_ab_right, 14);
	insertBidirectionalEdge(named_region::identifier_upper_ab_left, named_region::identifier_chest_right, 17);

	//Right Upper_ Ab
	insertBidirectionalEdge(named_region::identifier_upper_ab_right, named_region::identifier_chest_right, 10);
	insertBidirectionalEdge(named_region::identifier_upper_ab_right, named_region::identifier_middle_ab_right, 10);
	insertBidirectionalEdge(named_region::identifier_upper_ab_right, named_region::identifier_upper_ab_left, 10);
	insertBidirectionalEdge(named_region::identifier_upper_ab_right, named_region::identifier_middle_ab_left, 14);
	insertBidirectionalEdge(named_region::identifier_upper_ab_right, named_region::identifier_chest_left, 17);

	//Left Mid_ Ab 
	insertBidirectionalEdge(named_region::identifier_middle_ab_left, named_region::identifier_upper_ab_left, 10);
	insertBidirectionalEdge(named_region::identifier_middle_ab_left, named_region::identifier_lower_ab_left, 10);
	insertBidirectionalEdge(named_region::identifier_middle_ab_left, named_region::identifier_upper_ab_right, 14);
	insertBidirectionalEdge(named_region::identifier_middle_ab_left, named_region::identifier_middle_ab_right, 10);
	insertBidirectionalEdge(named_region::identifier_middle_ab_left, named_region::identifier_lower_ab_right, 14);

	//Right Mid_ Ab 
	insertBidirectionalEdge(named_region::identifier_middle_ab_right, named_region::identifier_upper_ab_right, 10);
	insertBidirectionalEdge(named_region::identifier_middle_ab_right, named_region::identifier_lower_ab_right, 10);
	insertBidirectionalEdge(named_region::identifier_middle_ab_right, named_region::identifier_upper_ab_left, 14);
	insertBidirectionalEdge(named_region::identifier_middle_ab_right, named_region::identifier_middle_ab_left, 10);
	insertBidirectionalEdge(named_region::identifier_middle_ab_right, named_region::identifier_lower_ab_left, 14);

	//Left Lower_ Ab
	insertBidirectionalEdge(named_region::identifier_lower_ab_left, named_region::identifier_lower_ab_right, 10);
	insertBidirectionalEdge(named_region::identifier_lower_ab_left, named_region::identifier_middle_ab_right, 14);
	insertBidirectionalEdge(named_region::identifier_lower_ab_left, named_region::identifier_middle_ab_left, 10);

	//Right Lower_ Ab   
	insertBidirectionalEdge(named_region::identifier_lower_ab_right, named_region::identifier_lower_ab_left, 10);
	insertBidirectionalEdge(named_region::identifier_lower_ab_right, named_region::identifier_middle_ab_left, 14);
	insertBidirectionalEdge(named_region::identifier_lower_ab_right, named_region::identifier_middle_ab_right, 10);

	//Left shoulder
	insertBidirectionalEdge(named_region::identifier_shoulder_left, named_region::identifier_chest_left, 10);
	insertBidirectionalEdge(named_region::identifier_shoulder_left, named_region::identifier_upper_back_left, 20);
	insertBidirectionalEdge(named_region::identifier_shoulder_left, named_region::identifier_upper_arm_left, 10);

	//Right shoulder
	insertBidirectionalEdge(named_region::identifier_shoulder_right, named_region::identifier_chest_right, 10);
	insertBidirectionalEdge(named_region::identifier_shoulder_right, named_region::identifier_upper_back_right, 20);
	insertBidirectionalEdge(named_region::identifier_shoulder_right, named_region::identifier_upper_arm_right, 10);

	//Left upper arm
	insertBidirectionalEdge(named_region::identifier_upper_arm_left, named_region::identifier_shoulder_left, 10);
	insertBidirectionalEdge(named_region::identifier_upper_arm_left, named_region::identifier_lower_arm_left, 10);

	//Right upper arm
	insertBidirectionalEdge(named_region::identifier_upper_arm_right, named_region::identifier_shoulder_right, 10);
	insertBidirectionalEdge(named_region::identifier_upper_arm_right, named_region::identifier_lower_arm_right, 10);

	//Left forearm
	insertBidirectionalEdge(named_region::identifier_lower_arm_left, named_region::identifier_upper_arm_left, 10);

	//Right forearm
	insertBidirectionalEdge(named_region::identifier_lower_arm_right, named_region::identifier_upper_arm_right, 10);

	//Left back
	insertBidirectionalEdge(named_region::identifier_upper_back_left, named_region::identifier_shoulder_left, 20);
	insertBidirectionalEdge(named_region::identifier_upper_back_left, named_region::identifier_upper_back_right, 20);

	//Right back
	insertBidirectionalEdge(named_region::identifier_upper_back_right, named_region::identifier_shoulder_right, 20);
	insertBidirectionalEdge(named_region::identifier_upper_back_right, named_region::identifier_upper_back_left, 20);
}

void PathFinder::insertBidirectionalEdge(named_region from, named_region to, int weight)
{
	auto& fromNeighbors = m_edges[from];
	if (std::find(fromNeighbors.begin(), fromNeighbors.end(), to) == fromNeighbors.end()) {
		fromNeighbors.push_back(to);
	}

	auto& toNeighbors = m_edges[to];
	if (std::find(toNeighbors.begin(), toNeighbors.end(), from) == toNeighbors.end()) {
		toNeighbors.push_back(from);
	}

	m_weights[from][to] = weight;
	m_weights[to][from] = weight;
}

int PathFinder::cost(named_region from, named_region to)
{
	return m_weights[from][to];
}
