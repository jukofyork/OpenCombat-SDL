#pragma once

#include <ai/Path.h>
#include <vector>
#include <cstddef>
#include <limits>
#include <unordered_map>
#include <cassert>
#include <world/Element.h>
#include <algorithm>

/**
 * A* pathfinding implementation using std::vector and indices only.
 * No raw pointers - completely pointer-free design.
 */
class AStar
{
public:
	AStar(void);
	virtual ~AStar(void);

	// Finds a path from (x0,y0) to (x1,y1), using the method
	// (prone, med, etc) found in level
	Path *FindPath(int x0, int y0, int x1, int y1, Element::Level level);

protected:
	// Node structure - all relationships use indices
	struct Node
	{
		int X, Y;
		float F, G, H;
		size_t ParentIdx;  // max value if no parent
		size_t HeapIndex;  // Position in the heap
		bool InOpenSet;
		bool InClosedSet;

		Node() : X(0), Y(0), F(0), G(0), H(0), 
		         ParentIdx(std::numeric_limits<size_t>::max()), 
		         HeapIndex(std::numeric_limits<size_t>::max()), 
		         InOpenSet(false), InClosedSet(false) {}
	};

	// Packs two 32-bit coordinates into a 64-bit key for unordered_map
	struct CoordinatePacker {
		size_t operator()(const std::pair<int, int>& p) const {
			return (static_cast<size_t>(p.first) << 32) | static_cast<size_t>(p.second);
		}
	};

	// Gets the cost of moving to tile (x,y)
	float GetTerrainCost(int x, int y);

	// Generates a heuristic for the cost remaining
	float Heuristic(int x, int y);

	// Binary min-heap for open set
	std::vector<size_t> _openHeap;
	
	// All nodes stored here
	std::vector<Node> _nodes;
	
	// Map from (x,y) to node index for quick lookup
	std::unordered_map<std::pair<int, int>, size_t, CoordinatePacker> _nodeMap;
	
	// Destination
	int _destX, _destY;
	
	// Current level
	Element::Level _level;

	// Heap operations
	void HeapPush(size_t nodeIdx);
	size_t HeapPop();
	void HeapSiftUp(size_t heapIdx);
	void HeapSiftDown(size_t heapIdx);
	void HeapUpdate(size_t heapIdx);
};