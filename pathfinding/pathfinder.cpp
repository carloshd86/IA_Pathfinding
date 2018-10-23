#include <stdafx.h>

#include "pathfinder.h"
#include <algorithm>
#include <functional>


const int Pathfinder::numDirections = 4;
const int Pathfinder::dirX[numDirections] = { 1, 0, -1, 0 };
const int Pathfinder::dirY[numDirections] = { 0, 1, 0, -1 };

Pathfinder::Pathfinder() : MOAIEntity2D(),
	mGridRows(0),
	mGridCols(0)
{
	RTTI_BEGIN
		RTTI_EXTEND(MOAIEntity2D)
	RTTI_END

	ReadPath("grid.txt", "pathcost.txt");
}

Pathfinder::~Pathfinder()
{

}

void Pathfinder::UpdatePath()
{
	mPath.clear();
	Astar();
}

void Pathfinder::ReadPath(const char* gridFilename, const char* pathCostFilename)
{
	std::ifstream pathCostFile(pathCostFilename, std::ios::binary);
	if (pathCostFile.is_open()) {

		std::map<char, int> pathCosts;

		std::string line;
		char key;
		std::string value;
		while (!pathCostFile.eof()) {
			std::getline(pathCostFile, line);
			int index = line.find('=', 0);
			// Assuming each cost is represented by only one char
			key = line.substr(0, index)[0];
			value = line.substr(index + 1);
			value.erase(std::remove(value.begin(), value.end(), '\r'), value.end());
			value.erase(std::remove(value.begin(), value.end(), '\n'), value.end());
			pathCosts[key] = std::stoi(value);
		}

		std::ifstream pathFile(gridFilename, std::ios::binary);
		if (pathFile.is_open()) {
			std::string line;
			size_t lineIndex = 0;
			while (!pathFile.eof()) {
				std::getline(pathFile, line);
				line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
				line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
				size_t lineLength = line.length();
				if (mGridCols != lineLength) {
					mGridCols = lineLength;
				}
				for (size_t i = 0; i < lineLength; ++i) {
					if (pathCosts.end() != pathCosts.find(line[i])) {
						mGrid[GridNode(i, lineIndex)] = pathCosts[line[i]];
					} else {
						// Unreachable position
						mGrid[GridNode(i, lineIndex)] = -1;
					}
				}
				++lineIndex;
				mGridRows = lineIndex;
			}
		}
	}
}

void Pathfinder::Astar()
{
	GridNode startNode = GetNodeFromScreenPosition(mStartPosition);
	GridNode endNode = GetNodeFromScreenPosition(mEndPosition);

	if (IsGridNodeValid(startNode) && IsGridNodeValid(endNode) && !startNode.Compare(endNode)) {
		std::vector<PathNode*> openList;
		std::vector<PathNode*> closedList;

		PathNode* pathNode = new PathNode(startNode);
		openList.push_back(pathNode);

		while (!openList.empty()) {
			pathNode = *openList.begin();
			openList.erase(openList.begin());
			closedList.push_back(pathNode);

			if (endNode.Compare(pathNode->node)) {
				// Node is the end node
				BuildPath(*pathNode);

				// Releasing memory for stored nodes
				for (PathNode* toDelete : openList) {
					delete toDelete;
				}
				for (PathNode* toDelete : closedList) {
					delete toDelete;
				}
				return;
			} else {
				// Node is not the end node
				std::vector<PathNode*> connections;
				GetNodeConnections(*pathNode, connections);
				for (PathNode* nextPathNode : connections) {
					if (closedList.end() != std::find_if(closedList.begin(), closedList.end(), std::bind(&PathNode::CompareNodePointer, std::placeholders::_1, nextPathNode))) {
						// Node already on closedList
						continue;
					} else {
						auto& openListNodeFound = std::find_if(openList.begin(), openList.end(), std::bind(&PathNode::CompareNodePointer, std::placeholders::_1, nextPathNode));
						if (openList.end() != openListNodeFound) {
							// Change cost and parent if cost is smaller
							if (nextPathNode->g < (*openListNodeFound)->g) {
								(*openListNodeFound)->g = nextPathNode->g;
								(*openListNodeFound)->parent = nextPathNode->parent;
							}
							
							// Releasing the node memory as it is nor in openList neither in closedList
							delete nextPathNode;
						} else {
							// Adding the pathNode to openList
							openList.push_back(nextPathNode);
						}
					}
				}
			}
		}
		
		// Releasing memory for stored nodes
		for (PathNode* toDelete : openList) {
			delete toDelete;
		}
		for (PathNode* toDelete : closedList) {
			delete toDelete;
		}
	}
}

void Pathfinder::GetNodeConnections(const PathNode& pathNode, std::vector<PathNode*>& connections) {
	connections.clear();

	for (int i = 0; i < numDirections; ++i) {
		GridNode nextNode(pathNode.node.x + dirX[i], pathNode.node.y + dirY[i]);
		if (IsGridNodeValid(nextNode)) {
			int cost = pathNode.g + mGrid[nextNode];
			connections.push_back(new PathNode(nextNode, cost, cost, &pathNode));
		}
	}
}

bool Pathfinder::IsGridNodeValid(const GridNode& node) const {
	// Returns true if the node is within the limits of the grid and has a valid cost (reachable node)
	return node.x >= 0 && node.y >= 0 && node.x < static_cast<int>(mGridCols) && node.y < static_cast<int>(mGridRows) && mGrid.end() != mGrid.find(node) && mGrid.at(node) >= 0;
}

void Pathfinder::BuildPath(const PathNode& lastNode) {
	mPath.clear();
	mPath.push_back(lastNode.node);
	const PathNode* parentNode = lastNode.parent;
	while (parentNode) {
		mPath.push_back(parentNode->node);
		parentNode = parentNode->parent;
	}
	std::reverse(mPath.begin(), mPath.end());
}

GridNode Pathfinder::GetNodeFromScreenPosition(USVec2D& screenPosition) {
	GridNode result(0, 0);
	if (mGridRows && mGridCols) {
		int left = -512;
		int top = -384;
		int colWidth = 1024/mGridCols;
		int rowHeight = 768/mGridRows;
		result.x = (screenPosition.mX - left) / colWidth;
		result.y = (screenPosition.mY - top) / rowHeight;
	}
	return result;
}

void Pathfinder::DrawDebug()
{
	MOAIGfxDevice& gfxDevice = MOAIGfxDevice::Get();

	if (mGridRows && mGridCols) {
		int left = -512;
		int top = -384;
		int colWidth = 1024/mGridCols;
		int rowHeight = 768/mGridRows;

		for (int x = 0; x < static_cast<int>(mGridCols); ++x) {
			int pointLeft = x * colWidth + left;
			for (int y = 0; y < static_cast<int>(mGridRows); ++y) {
				int pointTop = y * rowHeight + top;
				
				GridNode currentNode(x, y);
				if (IsGridNodeValid(currentNode)) {
					switch (mGrid[currentNode]) {
					case 1:
						gfxDevice.SetPenColor(0.0f, 0.0f, 0.9f, 0.2f);
						break;
					case 2:
						gfxDevice.SetPenColor(0.0f, 0.0f, 0.6f, 0.2f);
						break;
					case 3:
						gfxDevice.SetPenColor(0.0f, 0.0f, 0.3f, 0.2f);
						break;
					case 4:
						gfxDevice.SetPenColor(0.0f, 0.0f, 0.1f, 0.2f);
						break;
					default:
						gfxDevice.SetPenColor(0.0f, 0.0f, 0.9f, 0.2f);
						break;
					}
					
					MOAIDraw::DrawRectFill(pointLeft, pointTop, pointLeft + colWidth, pointTop + rowHeight);
					gfxDevice.SetPenColor(0.0f, 0.0f, 0.0f, 1.0f);
					MOAIDraw::DrawRectOutline(pointLeft, pointTop, pointLeft + colWidth, pointTop + rowHeight);
				} else {
					gfxDevice.SetPenColor(0.5f, 0.5f, 0.5f, 0.5f);
					MOAIDraw::DrawRectFill(pointLeft, pointTop, pointLeft + colWidth, pointTop + rowHeight);
					gfxDevice.SetPenColor(0.0f, 0.0f, 0.0f, 1.0f);
					MOAIDraw::DrawRectOutline(pointLeft, pointTop, pointLeft + colWidth, pointTop + rowHeight);
				}
			}
		}

		if (!mPath.empty()) {
			for (GridNode& node : mPath) {
				int pointLeft = node.x * colWidth + left;
				int pointTop = node.y * rowHeight + top;
				gfxDevice.SetPenColor(1.0f, 0.0f, 0.0f, 0.75f);
				MOAIDraw::DrawRectFill(pointLeft, pointTop, pointLeft + colWidth, pointTop + rowHeight);
				gfxDevice.SetPenColor(0.0f, 0.0f, 0.0f, 1.0f);
				MOAIDraw::DrawRectOutline(pointLeft, pointTop, pointLeft + colWidth, pointTop + rowHeight);
			}
		}

		GridNode startNode = GetNodeFromScreenPosition(mStartPosition);
		gfxDevice.SetPenColor(1.0f, 1.0f, 1.0f, 0.75f);
		int startPointLeft = startNode.x * colWidth + left;
		int startPointTop = startNode.y * rowHeight + top;
		MOAIDraw::DrawRectOutline(startPointLeft, startPointTop, startPointLeft + colWidth, startPointTop + rowHeight);

		GridNode endNode = GetNodeFromScreenPosition(mEndPosition);
		gfxDevice.SetPenColor(0.0f, 0.75f, 0.0f, 0.75f);
		int endPointLeft = endNode.x * colWidth + left;
		int endPointTop = endNode.y * rowHeight + top;
		MOAIDraw::DrawRectOutline(endPointLeft, endPointTop, endPointLeft + colWidth, endPointTop + rowHeight);

	}
}

bool Pathfinder::PathfindStep()
{
    // returns true if pathfinding process finished
    return true;
}















//lua configuration ----------------------------------------------------------------//
void Pathfinder::RegisterLuaFuncs(MOAILuaState& state)
{
	MOAIEntity::RegisterLuaFuncs(state);

	luaL_Reg regTable [] = {
		{ "setStartPosition",		_setStartPosition},
		{ "setEndPosition",			_setEndPosition},
        { "pathfindStep",           _pathfindStep},
		{ NULL, NULL }
	};

	luaL_register(state, 0, regTable);
}

int Pathfinder::_setStartPosition(lua_State* L)
{
	MOAI_LUA_SETUP(Pathfinder, "U")

	float pX = state.GetValue<float>(2, 0.0f);
	float pY = state.GetValue<float>(3, 0.0f);
	self->SetStartPosition(pX, pY);
	return 0;
}

int Pathfinder::_setEndPosition(lua_State* L)
{
	MOAI_LUA_SETUP(Pathfinder, "U")

	float pX = state.GetValue<float>(2, 0.0f);
	float pY = state.GetValue<float>(3, 0.0f);
	self->SetEndPosition(pX, pY);
	return 0;
}

int Pathfinder::_pathfindStep(lua_State* L)
{
    MOAI_LUA_SETUP(Pathfinder, "U")

    self->PathfindStep();
    return 0;
}