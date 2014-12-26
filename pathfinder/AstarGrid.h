#pragma once
#include "AstarContainers.h"

typedef node_heap PfOpenList;
//typedef node_vect PfOpenList;

struct AstarGrid
{
	AstarNode* Nodes;	// Array of all nodes

	int Width, Height;	// size of this 'grid world'
	//AstarNode* Goal;	// goal of the path (destination)
	uint NumPlanes;		// number of planes in this grid

	inline AstarGrid() : Nodes(0), Width(0), Height(0) {}
	inline ~AstarGrid() { destroy(); }

	/**
	 * @brief Completely destroy all allocated data
	 *        You must call create() again to initialize the grid
	 */
	void destroy();

	/**
	 * @brief Initializes AstarGrid from 2D 1-channel bitmap data
	 *        with the specified dimensions
	 */
	void create(int width, int height, const byte* initData);
	int fill_planes(int count, int firstPlane);
	void quick_fill(PfVector<AstarNode*>& open, AstarNode* node, int plane);


	//// @brief This is only used during init methods
	AstarNode* get(int x, int y);

	/**
	 * @brief Opens the specified node. Expects a gain parameter
	 */
	bool open(AstarNode* prev, AstarNode& n, int gain);
};