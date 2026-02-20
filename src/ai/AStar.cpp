#include "./AStar.h"
#include <cmath>
#include <limits>
#include <application/Globals.h>

AStar::AStar(void)
{
	_destX = -1;
	_destY = -1;
}

AStar::~AStar(void)
{
}

float
AStar::Heuristic(int x, int y)
{
	// Weight diagonals slightly more
	int dx = std::abs(x - _destX);
	int dy = std::abs(y - _destY);
	float diag = static_cast<float>(std::min(dx, dy));
	float straight = static_cast<float>(dx + dy);
	return sqrtf(2.0f) * diag + (straight - 2.0f * diag);
}

float
AStar::GetTerrainCost(int x, int y)
{
	Element *e = g_Globals->World.CurrentWorld->GetTileElement(x, y);
	return static_cast<float>(e->Hindrance[_level]) / 100.0f;
}

void
AStar::HeapPush(size_t nodeIdx)
{
	_openHeap.push_back(nodeIdx);
	_nodes[nodeIdx].InOpenSet = true;
	HeapSiftUp(_openHeap.size() - 1);
}

size_t
AStar::HeapPop()
{
	assert(!_openHeap.empty());
	
	size_t result = _openHeap[0];
	_nodes[result].InOpenSet = false;
	
	// Move last to front and sift down
	_openHeap[0] = _openHeap.back();
	_nodes[_openHeap[0]].HeapIndex = 0;
	_openHeap.pop_back();
	
	if (!_openHeap.empty()) {
		HeapSiftDown(0);
	}
	
	return result;
}

void
AStar::HeapSiftUp(size_t heapIdx)
{
	_nodes[_openHeap[heapIdx]].HeapIndex = heapIdx;
	
	while (heapIdx > 0) {
		size_t parentIdx = (heapIdx - 1) / 2;
		if (_nodes[_openHeap[parentIdx]].F <= _nodes[_openHeap[heapIdx]].F) {
			break;
		}
		
		// Swap with parent
		std::swap(_openHeap[parentIdx], _openHeap[heapIdx]);
		_nodes[_openHeap[parentIdx]].HeapIndex = parentIdx;
		_nodes[_openHeap[heapIdx]].HeapIndex = heapIdx;
		
		heapIdx = parentIdx;
	}
}

void
AStar::HeapSiftDown(size_t heapIdx)
{
	_nodes[_openHeap[heapIdx]].HeapIndex = heapIdx;
	
	while (true) {
		size_t leftChild = 2 * heapIdx + 1;
		size_t rightChild = 2 * heapIdx + 2;
		size_t smallest = heapIdx;
		
		if (leftChild < _openHeap.size() && 
		    _nodes[_openHeap[leftChild]].F < _nodes[_openHeap[smallest]].F) {
			smallest = leftChild;
		}
		
		if (rightChild < _openHeap.size() && 
		    _nodes[_openHeap[rightChild]].F < _nodes[_openHeap[smallest]].F) {
			smallest = rightChild;
		}
		
		if (smallest == heapIdx) {
			break;
		}
		
		// Swap with smallest child
		std::swap(_openHeap[heapIdx], _openHeap[smallest]);
		_nodes[_openHeap[heapIdx]].HeapIndex = heapIdx;
		_nodes[_openHeap[smallest]].HeapIndex = smallest;
		
		heapIdx = smallest;
	}
}

Path *
AStar::FindPath(int x0, int y0, int x1, int y1, Element::Level level)
{
	_level = level;
	_destX = x1;
	_destY = y1;
	
	// Clear previous state
	_nodes.clear();
	_nodeMap.clear();
	_openHeap.clear();
	
	// Create start node
	Node startNode;
	startNode.X = x0;
	startNode.Y = y0;
	startNode.G = 0;
	startNode.H = Heuristic(x0, y0);
	startNode.F = startNode.G + startNode.H;
	startNode.ParentIdx = std::numeric_limits<size_t>::max();
	
	_nodes.push_back(startNode);
	_nodeMap[{x0, y0}] = 0;
	HeapPush(0);
	
	// Directions: 8-connected grid
	const int dx[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
	const int dy[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
	const float moveCost[8] = {1.414f, 1.0f, 1.414f, 1.0f, 1.0f, 1.414f, 1.0f, 1.414f};
	
	while (!_openHeap.empty()) {
		size_t currentIdx = HeapPop();
		Node& current = _nodes[currentIdx];
		current.InClosedSet = true;
		
		// Check if we reached the destination
		if (current.X == x1 && current.Y == y1) {
			// Reconstruct path
			Path *path = nullptr;
			size_t idx = currentIdx;
			
			while (idx != std::numeric_limits<size_t>::max()) {
				Node& n = _nodes[idx];
				Path *p = AllocatePath();
				p->X = n.X;
				p->Y = n.Y;
				p->Next = path;
				path = p;
				idx = n.ParentIdx;
			}
			return path;
		}
		
		// Generate successors
		for (int i = 0; i < 8; ++i) {
			int nx = current.X + dx[i];
			int ny = current.Y + dy[i];
			
			// Check bounds and passability
			if (nx < 0 || ny < 0 || 
			    nx >= g_Globals->World.CurrentWorld->NumTiles.x || 
			    ny >= g_Globals->World.CurrentWorld->NumTiles.y) {
				continue;
			}
			
			if (!g_Globals->World.CurrentWorld->IsPassable(nx, ny)) {
				continue;
			}
			
			// Calculate new G score
			float tentativeG = current.G + GetTerrainCost(nx, ny) * moveCost[i];
			
			// Check if this node exists
			auto it = _nodeMap.find({nx, ny});
			if (it != _nodeMap.end()) {
				size_t neighborIdx = it->second;
				Node& neighbor = _nodes[neighborIdx];
				
				if (neighbor.InClosedSet) {
					continue;
				}
				
				if (tentativeG < neighbor.G) {
					// Better path found
					neighbor.ParentIdx = currentIdx;
					neighbor.G = tentativeG;
					neighbor.F = neighbor.G + neighbor.H;
					
					if (neighbor.InOpenSet) {
						// Update position in heap
						HeapSiftUp(neighbor.HeapIndex);
					} else {
						HeapPush(neighborIdx);
					}
				}
			} else {
				// Create new node
				Node newNode;
				newNode.X = nx;
				newNode.Y = ny;
				newNode.G = tentativeG;
				newNode.H = Heuristic(nx, ny);
				newNode.F = newNode.G + newNode.H;
				newNode.ParentIdx = currentIdx;
				
				size_t newIdx = _nodes.size();
				_nodes.push_back(newNode);
				_nodeMap[{nx, ny}] = newIdx;
				HeapPush(newIdx);
			}
		}
	}
	
	// No path found
	return nullptr;
}