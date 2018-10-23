#include <stdafx.h>

#include "PathNode.h"

bool PathNode::CompareNode(const PathNode& pathNode1, const PathNode& pathNode2) {
	return pathNode1.node.Compare(pathNode2.node);
}