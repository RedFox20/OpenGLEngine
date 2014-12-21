/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#pragma once
#ifndef PATHFINDER_ASTAR_H
#define PATHFINDER_ASTAR_H


#include "Basetypes.h"
#include <forward_list>
using std::forward_list;
#include <deque>
using std::deque;
#include "GLDraw.h"
#include "PfList.h"
#include "PfVector.h"

extern Vector2 gScreen; // global screen size

#define USE_JPS 0

#define USE_PFLIST 0
#if !USE_PFLIST
#define USE_PFVECTOR 1
#endif

struct AstarNode 
{
	ushort X, Y;		// the X, Y position in the world

	ushort ID;			// ID of this node in the 'world' (actually just an array index)
	byte Plane;			// plane of this Node, collision plane is always 1
	bool IsClosed;		// true if this node has been closed
	
	ushort GScore;		// accumulated sum of G values g=(10 or 14
	ushort FScore;		// F = H(manhattan) + GScore score to destination

	uint OpenID;		// ID of an 'open' node, this is used to check if we've opened this node or not
	AstarNode* Prev;	// previous node in this path
	PfList<AstarNode*>::iterator OpenNode;// PfList node for the open list

	AstarNode *N, *S, *W, *E;
	AstarNode *NW, *NE, *SW, *SE;

	bool IsReopened;	// true if this node has been reopened



	void Init(int id, int x, int y, int plane)
	{
		ID = id, X = x, Y = y, Plane = plane;
		OpenID = 0;
		IsClosed = false;
#if !USE_PFLIST
		IsReopened = false;
#endif
		Prev = NULL;
		FScore = 0;
		GScore = 0;
	}
	void Clear()
	{
		GScore = 0, FScore = 0;
	}
	bool Open(AstarNode* end, AstarNode* prev, uint openID, int gain)
	{
		if(prev->Plane != Plane) return false; // not on this plane
		if(openID == OpenID) // we have opened this Node before
		{
			if(IsClosed) return false; // don't touch it if it's CLOSED
			ushort G = prev->GScore + gain;
			if(G >= GScore) return false; // if the new gain is worse, then don't touch it
			GScore = G;
			FScore = 5*(std::abs(end->X - X) + std::abs(end->Y - Y)) + G;
#if !USE_PFLIST
			IsReopened = true;
#endif
			Prev = prev;
			return true; // reopened
		}
#if USE_PFLIST
		OpenNode = NULL;
#else
		IsReopened = false;
#endif
		OpenID = openID, IsClosed = false, Prev = prev;
		GScore = prev->GScore + gain;
		FScore = 5*(std::abs(end->X - X) + std::abs(end->Y - Y)) + GScore;
		return true; // opened
	}
	void Close()
	{
		IsClosed = true;
	}
};




struct AstarGrid
{
	AstarNode* Nodes;	// Array of all nodes
	int Width, Height;	// size of this 'grid world'
	AstarNode* Goal;	// goal of the path (destination)
	uint OpenID;		// unique ID to use when opening new grids
	uint NumPlanes;		// number of planes in this grid

	AstarGrid() : Nodes(0), Width(0), Height(0), Goal(0), OpenID(0) {}
	~AstarGrid() { Destroy(); }

	void Destroy()
	{
		if(Nodes) {
			delete Nodes, Nodes = 0;
			Width = 0, Height = 0;
			Goal = 0, OpenID = 0;
		}
	}
	void Create(int width, int height, const byte* initData)
	{
		int count = width * height;
		Width = width, Height = height;
		Nodes = new AstarNode[count];
		for(int x = 0; x < width; x++) 
		{
			for(int y = 0; y < height; y++)
			{
				int i = (x * width) + y;
				int plane = initData[i] < 128 ? 1 : 0; // black tiles: plane1, other uninit
				Nodes[i].Init(i, x, y, plane);
			}
		}

		NumPlanes = FillPlanes(count, 2);
	}

	int FillPlanes(int count, int firstPlane);
	void QuickFill(PfVector<AstarNode*>& open, AstarNode* node, int plane);

	AstarNode* Get(int x, int y)
	{
		if(x < 0 || Width <= x || y < 0 || Height <= y) return NULL; // world bounds checkin'
		return &Nodes[x * Width + y];
	}

	void Begin(AstarNode* start, AstarNode* goal)
	{
		start->Prev = NULL;
		Goal = goal;
		++OpenID; // with 1000 * 60 pathfinds per second, this will overflow in: ~8.17 years
	}

	AstarNode* Open(AstarNode* head, int dirX, int dirY)
	{
		AstarNode* node = Get(head->X + dirX, head->Y + dirY);
		if(!node || node == head->Prev) // avoid NULL & circular references
			return NULL;
		if(node->Open(Goal, head, OpenID, ((dirX && dirY) ? 7 : 5))) // is it diagonal ? use SQRT2*5
			return node;
		return NULL;
	}
};




struct AstarQNode
{
	typedef AstarQNode QNode;
	typedef AstarNode ANode;
	static const size_t MAXCHILDREN = 4; // max children per node
	int x, y;
	int size, size2;
	//PfList<AstarNode*> objects;
	AstarNode* object;
	QNode* parent;
	QNode* NW;
	QNode* NE;
	QNode* SW;
	QNode* SE;
	inline AstarQNode(QNode* parent, int x, int y, int size)
		: x(x), y(y), size(size), size2(size/2), object(NULL),
		parent(parent), NW(0), NE(0), SW(0), SE(0)
	{
	}
	QNode* NewNode();
	void DeleteNode(QNode* n);
	void Destroy()
	{
		if(NW) NW->Destroy(), DeleteNode(NW), NW = NULL;
		if(NE) NE->Destroy(), DeleteNode(NE), NE = NULL;
		if(SW) SW->Destroy(), DeleteNode(SW), SW = NULL;
		if(SE) SE->Destroy(), DeleteNode(SE), SE = NULL;
		//objects.clear();
	}
	void GetAllQNodes(vector<QNode*>& out)
	{
		out.push_back(this);
		if(NW) NW->GetAllQNodes(out);
		if(NE) NE->GetAllQNodes(out);
		if(SW) SW->GetAllQNodes(out);
		if(SE) SE->GetAllQNodes(out);
	}
	QNode* Get(int x, int y)
	{
		if(NW && NW_Contains(x, y)) return NW->Get(x, y);
		if(NE && NE_Contains(x, y)) return NE->Get(x, y);
		if(SW && SW_Contains(x, y)) return SW->Get(x, y);
		if(SE && SE_Contains(x, y)) return SE->Get(x, y);
		return this;
	}
	void GetNeighbouringTiles(QNode* root, PfVector<QNode*>& list)
	{
		QNode* n;
		if(n = root->Get(x - size2, y)) n->GetETiles(list); // move west and get all East tiles
		if(n = root->Get(x + size2, y)) n->GetWTiles(list); // move east and get all West tiles
		if(n = root->Get(x, y + size2)) n->GetSTiles(list); // move north and get all South tiles
		if(n = root->Get(x, y - size2)) n->GetNTiles(list); // move south and get all North tiles
		if(n = root->Get(x - size2, y + size2)) n->GetSETiles(list); // NW, get SE
		if(n = root->Get(x + size2, y + size2)) n->GetSWTiles(list); // NE, get SW
		if(n = root->Get(x - size2, y - size2)) n->GetNETiles(list); // SW, get NE
		if(n = root->Get(x + size2, y - size2)) n->GetNWTiles(list); // SE, get NW
	}
	void GetWTiles(PfVector<QNode*>& list)
	{
		if(NE) { NE->GetWTiles(list), SE->GetWTiles(list); return; } list.push_back(this);
	}
	void GetETiles(PfVector<QNode*>& list)
	{
		if(NW) { NW->GetETiles(list), SW->GetETiles(list); return; } list.push_back(this);
	}
	void GetNTiles(PfVector<QNode*>& list)
	{
		if(NW) { NW->GetNTiles(list), SW->GetNTiles(list); return; } list.push_back(this);
	}
	void GetSTiles(PfVector<QNode*>& list)
	{
		if(SW) { SW->GetSTiles(list), SE->GetSTiles(list); return; } list.push_back(this);
	}
	void GetNWTiles(PfVector<QNode*>& list)
	{
		if(NW) { NW->GetETiles(list); return; } list.push_back(this);
	}
	void GetNETiles(PfVector<QNode*>& list)
	{
		if(NE) { NE->GetWTiles(list); return; } list.push_back(this);
	}
	void GetSWTiles(PfVector<QNode*>& list)
	{
		if(SW) { SW->GetNTiles(list); return; }	list.push_back(this);
	}
	void GetSETiles(PfVector<QNode*>& list)
	{
		if(SE) { SE->GetSTiles(list); return; }	list.push_back(this);
	}



	void AddNode(ANode* n)
	{
		if(size == 1)
		{
			object = n;
			return;
		}

		if(!NW) NW = new (NewNode()) QNode(this, x, y + size2, size2);
		if(!NE) NE = new (NewNode()) QNode(this, x + size2, y + size2, size2);
		if(!SW) SW = new (NewNode()) QNode(this, x, y, size2);
		if(!SE) SE = new (NewNode()) QNode(this, x + size2, y, size2);
		
		int nx = n->X, ny = n->Y;
		if(NW_Contains(nx, ny))	
		{
			NW->AddNode(n);
			return;
		}
		else if(NE_Contains(nx, ny)) 
		{
			NE->AddNode(n);
			return;
		}
		else if(SW_Contains(nx, ny)) 
		{ 
			SW->AddNode(n);
			return;
		}
		else if(SE_Contains(nx, ny))
		{
			SE->AddNode(n);
			return;
		}
	}


	// true if the NW subnode contains the node(x,y)
	inline bool NW_Contains(int nx, int ny)
	{
		int px = x, py = y + size2;
		return Sub_Contains(px, py, nx, ny);
	}
	inline bool NE_Contains(int nx, int ny)
	{
		int px = x + size2, py = y + size2;
		return Sub_Contains(px, py, nx, ny);
	}
	inline bool SW_Contains(int nx, int ny)
	{
		int px = x, py = y;
		return Sub_Contains(px, py, nx, ny);
	}
	inline bool SE_Contains(int nx, int ny)
	{
		int px = x + size2, py = y;
		return Sub_Contains(px, py, nx, ny);
	}
	// true if a subnode at p(x,y) contains
	inline bool Sub_Contains(int px, int py, int nx, int ny)
	{
		return px <= nx && nx < (px + size2) && py <= ny && ny < (py + size2);
	}
};


struct AstarQGrid : public AstarGrid
{
	AstarQNode* Qtree;

	AstarQGrid() : Qtree(0) {}
	~AstarQGrid() { Destroy(); }

	void Destroy()
	{
		AstarGrid::Destroy();
		if(Qtree) delete Qtree, Qtree = 0;
	}
	void Create(int width, int height, const byte* initData)
	{
		AstarGrid::Create(width, height, initData);

		Qtree = new AstarQNode(NULL, 0, 0, upper_pow2(width));

		int count = width * height;
		for(int i = 0; i < count; ++i)
		{
			if(Nodes[i].Plane == 1)
			{
				Qtree->AddNode(&Nodes[i]);
			}
		}
	}
};


#define USE_QTREE 1


struct PathfinderAstar
{
#if USE_QTREE
	AstarQGrid grid;
#else
	AstarGrid grid;
#endif
	AstarNode* Start;	// start of the path
	AstarNode* End;		// end of the path (destination)
	float CellSize;
	float CellHalfSize; // CellSize / 2. Going to use this often, so better cache it
	int NumOpened;		// number of grids opened by the pathfinder
	int NumReopened;	// number of grids reopened by the pathfinder

#if USE_PFLIST
	PfList<AstarNode*> OpenList;
#elif USE_PFVECTOR
	PfVector<AstarNode*> OpenList;
#else
	vector<AstarNode*> OpenList;
#endif

	PathfinderAstar() : Start(0), End(0), CellSize(1.0f), CellHalfSize(0.5f), NumOpened(0), NumReopened(0) {}
	void Create(float cellSize, int width, int height, const byte* initData)
	{
		CellSize = cellSize;
		CellHalfSize = cellSize * 0.5f;
		grid.Create(width, height, initData);
#if USE_PFLIST
		OpenList.ResizeAllocator((width*height)/2); // resize the allocator to what we need
#endif
	}
	void Destroy()
	{
		grid.Destroy();
	}

	
	// converts a node's virtual lower-left position to screen position
	Vector2 ToScreenCoord(const AstarNode* node) const
	{
		return Vector2(node->X * CellSize, node->Y * CellSize);
	}

	// converts a node's virtual center position to screen position
	Vector2 ToScreenCoordCentered(const AstarNode* node) const
	{
		return Vector2(node->X * CellSize + CellHalfSize, node->Y * CellSize + CellHalfSize);
	}

	// converts a screen coordinate to virtual coordinates
	// only works correctly for positions that match InWorld()
	Vector2i ToVirtualCoord(const Vector2& pos) const
	{
		return Vector2i(int(pos.x / CellSize), int(pos.y / CellSize));
	}
	Vector2i ToVirtualCoord(float x, float y) const
	{
		return Vector2i(int(x / CellSize), int(y / CellSize));
	}
	bool InWorld(const Vector2& pos)
	{
		return 0.0f <= pos.x && pos.x < (grid.Width * CellSize) 
			&& 0.0f <= pos.y && pos.y < (grid.Width * CellSize);
	}


	inline bool SetStart(float worldX, float worldY) { return SetStart(Vector2(worldX, worldY)); }
	inline bool SetStart(int posX, int posY) { return SetStart(Vector2i(posX, posY)); }
	bool SetStart(const Vector2& worldXY) 
	{
		return InWorld(worldXY) ? SetStart(ToVirtualCoord(worldXY)) : false;
	}
	bool SetStart(const Vector2i& pos)
	{
		AstarNode* n = grid.Get(pos.x, pos.y);
		if(n && n != Start && n != End) { // not null && not same && not same as end
			Start = n, n->Clear();
			return true;
		}
		return false;
	}
	inline bool SetEnd(float worldX, float worldY) { return SetEnd(Vector2(worldX, worldY)); }
	inline bool SetEnd(int posX, int posY) { return SetEnd(Vector2i(posX, posY)); }
	bool SetEnd(const Vector2& worldXY) 
	{
		return InWorld(worldXY) ? SetEnd(ToVirtualCoord(worldXY)) : false;
	}
	bool SetEnd(const Vector2i& pos)
	{
		AstarNode* n = grid.Get(pos.x, pos.y);
		if(n && n != End && n != Start) { // not null && not same && not same as start
			End = n, n->Clear();
			return true;
		}
		return false;
	}


	/**
	 * This function inserts a node to an 'open' list based
	 * on the node's heuristic value
	 * The heuristic value is a rough 'scored' distance to destination
	 * Lower values are closer and will be inserted to the front of the list
	 */
	void InsertOpen(AstarNode* item)
	{
		++NumOpened;
#if USE_PFLIST
		if(OpenList.empty()){
			item->OpenNode = OpenList.push_front(item);
			return;
		}
		if(item->OpenNode) { // we know it's been reopened, so we need to remove it
			++NumReopened;
			OpenList.erase(item->OpenNode);
		}
		for(auto it = OpenList.begin(); it; ++it) {
			if((*it)->FScore >= item->FScore) {
				item->OpenNode = OpenList.insert_before(it, item);
				return;
			}
		}
		item->OpenNode = OpenList.push_back(item);
#elif USE_PFVECTOR
		if(OpenList.empty()) {
			OpenList.push_back(item);
			return;
		}
		if(item->IsReopened) {
			++NumReopened;
			auto before_begin = OpenList.before_begin(); // reverse iter end
			for(auto it = OpenList.before_end(); it != before_begin; --it) {
				if(*it == item) {
					OpenList.erase(it);
					break;
				}
			}
		}
		auto before_begin = OpenList.before_begin();
		for(auto it = OpenList.before_end(); it != before_begin; --it) {
			if((*it)->FScore >= item->FScore) {
				OpenList.insert_after(it, item);
				return;
			}
		}
		OpenList.insert(OpenList.begin(), item); // worst case scenario
#else
		if(OpenList.empty())
		{
			OpenList.push_back(item);
			return;
		}
		if(item->IsReopened)
		{
			++NumReopened;
			auto end = OpenList.end();
			for(auto it = OpenList.begin(); it != end; ++it) {
				if(*it == item) {
					OpenList.erase(it);
					break;
				}
			}
		}
		auto end = OpenList.end();
		for(auto it = OpenList.begin(); it != end; ++it) {
			if((*it)->FScore < item->FScore) {
				OpenList.insert(it, item);
				return;
			}
		}
		OpenList.push_back(item);
#endif
	}



#if USE_PFLIST || USE_PFVECTOR
	void Open(AstarNode* head, int dirX, int dirY, PfVector<Vector2>* explored)
#else
	void Open(AstarNode* head, int dirX, int dirY, vector<Vector2>* explored)

#endif
	{
		if(AstarNode* node = grid.Open(head, dirX, dirY)) 
		{
			InsertOpen(node);
			if(explored) 
			{
				explored->push_back(ToScreenCoordCentered(head));
				explored->push_back(ToScreenCoordCentered(node));
			}
		}
	}
#if USE_PFLIST || USE_PFVECTOR
	bool Process(PfVector<Vector2>& outPath, PfVector<Vector2>* explored = NULL)
#else
	bool Process(vector<Vector2>& outPath, vector<Vector2>* explored = NULL)
#endif
	{
		NumOpened = 0, NumReopened = 0;
		if(Start->Plane != End->Plane || Start->Plane == 1 || End->Plane == 1)
			return false; // no possible path between these two, or collision planes(1)
		grid.Begin(Start, End);

//#if USE_QTREE
		

#if 1
		AstarNode* head = Start;
		AstarNode* prev = head;
		while(head != End)
		{
#if USE_JPS
			if(head == prev)
			{
				Open(head, 0,  1, explored); // N
				Open(head, 1,  1, explored); // NE
				Open(head, 1,  0, explored); // E
				Open(head, 1, -1, explored); // SE
				Open(head, 0, -1, explored); // S
				Open(head, -1,-1, explored); // SW
				Open(head, -1, 0, explored); // W
				Open(head, -1, 1, explored); // NW
			}
			else
			{
				Vector2i dir(head->X - prev->X, head->Y - prev->Y);

				if(dir.x && dir.y) // diagonal
				{

				}
				else // horiz. / vert.
				{

				}
			}
#else
			Open(head, 0,  1, explored); // N
			Open(head, 1,  1, explored); // NE
			Open(head, 1,  0, explored); // E
			Open(head, 1, -1, explored); // SE
			Open(head, 0, -1, explored); // S
			Open(head, -1,-1, explored); // SW
			Open(head, -1, 0, explored); // W
			Open(head, -1, 1, explored); // NW
#endif


			// after inserting into the sorted list we get the heuristically best node available
			if(!OpenList.empty())
			{
#if USE_PFLIST
			// the first element is always the 'best'
				head = OpenList.pop_front();
				head->Close(); // mark it as 'Closed'
#elif USE_PFVECTOR
				OpenList.pop_back(head);
				head->Close();
#else
				head = OpenList.back();
				head->Close();
				OpenList.pop_back();
#endif
			}
			else break;
		}

#endif // USE_QTREE


		// construct the out path
		head = End;
		do { 
			outPath.push_back(ToScreenCoordCentered(head));
		} while(head = head->Prev);

		OpenList.clear(); // resets the pool
		return true;
	}
};


#endif // PATHFINDER_ASTAR_H