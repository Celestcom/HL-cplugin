#include "stdafx.h"
#include "RegionTree.h"


RegionNode::RegionNode(RegionNode* parent, std::string friendlyName) :
	parent(parent),
	friendlyName(friendlyName)
{
}


RegionNode* addChild(RegionNode& parent, std::string friendlyName) {
	auto ptr = std::make_unique<RegionNode>(&parent, friendlyName);
	auto addr = ptr.get();
	parent.children.push_back(std::move(ptr));

	return addr;
}
RegionTree::RegionTree() : m_root(nullptr, "body")
{
	auto left = addChild(m_root, "left");
	auto right = addChild(m_root, "right");

	addChild(*left, "left_back");
	addChild(*right, "right_back");

	auto left_arm = addChild(*left, "left_arm");
	auto right_arm = addChild(*right, "right_arm");

	addChild(*left, "left_shoulder");
	addChild(*right, "right_shoulder");

	addChild(*left_arm, "left_upper_arm");
	addChild(*left_arm, "left_forearm");

	addChild(*right_arm, "right_upper_arm");
	addChild(*right_arm, "right_forearm");

	auto left_chest = addChild(*left, "left_chest");
	auto right_chest = addChild(*right, "right_chest");

	addChild(*left_chest, "left_upper_chest");
	addChild(*left_chest, "left_upper_ab");
	addChild(*left_chest, "left_mid_ab");
	addChild(*left_chest, "left_lower_ab");

	addChild(*right_chest, "right_upper_chest");
	addChild(*right_chest, "right_upper_ab");
	addChild(*right_chest, "right_mid_ab");
	addChild(*right_chest, "right_lower_ab");
}

RegionNode* findRegion(RegionNode* root, const std::string& friendlyName) {
	if (root->friendlyName == friendlyName) {
		return root;
	}
	else {
		for (auto& child : root->children) {
			RegionNode* result = findRegion(child.get(), friendlyName);
			if (result != nullptr) {
				return result;
			}
		}
	}
	return nullptr;
}

bool RegionTree::Insert(const std::string & friendlyName, std::shared_ptr<HardwareDriver> driver)
{
	RegionNode* result = findRegion(&m_root, friendlyName);
	if (result != nullptr) {
		result->consumers.push_back(driver);
		return true;
	}

	return false;

}

bool RegionTree::Remove(const std::string & friendlyName, const std::shared_ptr<HardwareDriver>& driver)
{
	RegionNode* result = findRegion(&m_root, friendlyName);
	if (result != nullptr) {
		result->consumers.erase(std::find(result->consumers.begin(), result->consumers.end(), driver));
		return true;
	}

	return false;
}


const RegionTree::DriverList* findConsumerHelper(RegionNode* node) {
	if (node->consumers.empty()) {
		if (node->parent != nullptr) {
			return findConsumerHelper(node->parent);
		}
		else {
			return nullptr;
		}
	}
	else {
		return &node->consumers;
	}
}
const RegionTree::DriverList* RegionTree::Find(const std::string & friendlyName)
{
	RegionNode* result = findRegion(&m_root, friendlyName);
	if (result != nullptr) {
		return findConsumerHelper(result);
	}

	return nullptr;
}
