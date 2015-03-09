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
#include <xmmintrin.h>
#include "GLDraw.h"

#include "AstarGrid.h"

extern Vector2 gScreen; // global screen size

struct PathfinderAstar
{
	AstarGrid  Grid;
	AstarNode* Start;	// start of the path
	AstarNode* End;		// end of the path (destination)
	float CellSize;
	float CellHalfSize; // CellSize / 2. Going to use this often, so better cache it
	int NumOpened;		// number of grids opened by the pathfinder
	int NumReopened;	// number of grids reopened by the pathfinder
	int MaxDepth;       // max openlist depth
	uint OpenID;        // unique ID to use when opening new grids

	PfOpenList OpenList;

	inline PathfinderAstar() 
		: Start(0), End(0), CellSize(1.0f), CellHalfSize(0.5f), 
		  NumOpened(0), NumReopened(0), MaxDepth(0)
	{
	}
	void Create(float cellSize, int width, int height, const byte* initData);
	void Destroy();

	
	// converts a node's virtual lower-left position to screen position
	Vector2 ToScreenCoord(const AstarNode* node) const;

	// converts a node's virtual center position to screen position
	Vector2 ToScreenCoordCentered(const AstarNode* node) const;

	// converts a screen coordinate to virtual coordinates
	// only works correctly for positions that match InWorld()
	Vector2i ToVirtualCoord(const Vector2& pos) const;
	Vector2i ToVirtualCoord(float x, float y) const;
	bool InWorld(float x, float y) const;


	bool SetEnd(int x, int y);
	bool SetStart(int x, int y);
	bool SetEnd(const Vector2& worldXY);
	bool SetStart(const Vector2& worldXY);

	/**
	 * @brief Processes the current pathfinding request
	 * @note  Call SetStart() and SetEnd()
	 * @param outPath Resulting 
	 * @param explored List of explored paths in line pairs [A,B]; [B,C]; ...
	 */
	bool Process(PfVector<Vector2>& outPath, PfVector<Vector2>* explored = NULL);
};


#endif // PATHFINDER_ASTAR_H