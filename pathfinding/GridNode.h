#ifndef __GRIDNODE_H__
#define __GRIDNODE_H__

class GridNode {
public:
	GridNode(int x = 0, int y = 0) : x(x), y(y) {}

	int x;
	int y;

	bool Compare(const GridNode& other) const;
	bool operator==(const GridNode& other) const;
	bool operator<(const GridNode& other) const;
};

#endif