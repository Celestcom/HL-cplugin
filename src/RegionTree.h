#pragma once
#include "HardwareDriver.h"
#include <unordered_map>
#include <vector>
#include <memory>

class RegionNode {
public:
	std::vector<std::unique_ptr<RegionNode>> children;
	RegionNode* parent;
	RegionNode(RegionNode* parent, std::string friendlyName);
	std::string friendlyName;

	std::vector<std::shared_ptr<HardwareDriver>> consumers;
};


class RegionTree {
	
public:
	typedef std::vector<std::shared_ptr<HardwareDriver>> DriverList;
	RegionTree();
	bool Insert(const std::string& friendlyName, std::shared_ptr<HardwareDriver> driver);
	bool Remove(const std::string& friendlyName, const std::shared_ptr<HardwareDriver>& driver);
	const DriverList* Find(const std::string& friendlyName);
private:
	RegionNode m_root;
	std::vector<RegionNode*> m_leafView;
};
