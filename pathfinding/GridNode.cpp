#include <stdafx.h>

#include "GridNode.h"

bool GridNode::Compare(const GridNode& other) const {
	return other.x == x && other.y == y;
}

bool GridNode::operator==(const GridNode& other) const {
	return Compare(other);
}

bool GridNode::operator<(const GridNode& other) const {
	bool result = false;
	if (y == other.y) {
		return x < other.x;
	}
	else {
		return y < other.y;
	}
	return result;
}