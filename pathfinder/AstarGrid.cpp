#include "AstarGrid.h"

void AstarGrid::destroy()
{
	if (Nodes)
	{
		delete[] Nodes;
		Nodes = 0;
		Width = 0;
		Height = 0;
	}
}

void AstarGrid::create(int width, int height, const byte* initData)
{
	int count = width * height;
	Width  = width;
	Height = height;
	Nodes  = new AstarNode[count];

	for (int y = 0; y < height; ++y)
	for (int x = 0; x < width;  ++x) 
	{
		const int i = (y * width) + x;
		AstarNode& n = Nodes[i];
		n.FScore = 0;
		n.GScore = 0;
		n.HScore = 0;
		n.Closed = false;
		n.Plane  = initData[i] < 128 ? 1 : 0; // black tiles: plane1, other uninit
		n.X      = x;
		n.Y      = y;
		n.OpenID = 0;
		n.Prev   = 0;
		n.NumLinks = 0;
	}
	// initialize plane ID-s and grid links
	NumPlanes = fill_planes(count, 2);
}

int AstarGrid::fill_planes(int count, int firstPlane)
{
	PfVector<AstarNode*> open;
	open.reserve((Width + Height) * 2);
	for (int i = 0; i < count; ++i)
	{
		if (!Nodes[i].Plane) 
			quick_fill(open, &Nodes[i], firstPlane++);
	}
	return firstPlane;
}

void AstarGrid::quick_fill(PfVector<AstarNode*>& open, AstarNode* firstNode, int plane)
{
	// this is actually similar to the A* algo
	AstarNode* node = firstNode;
	while (true)
	{
		auto add = [&open, plane, node](AstarNode* link, int gain) {
			bool addlink = true;
			if (int count = node->NumLinks) {
				AstarLink* links = node->Links;
				for (int i = 0; i < count; ++i)
					if (links[i].node == link) { // link already exists?
						addlink = false;
						break;
					}
			}
			if (addlink)
				node->Links[node->NumLinks++] = { link, gain }; // add
			if (!link->Plane)
			{
				link->Plane = plane;
				open.push_back(link);
			}
		};
		
		int x = node->X, y = node->Y;
		if (AstarNode* N  = get(x    , y + 1)) add(N,  8);
		if (AstarNode* NE = get(x + 1, y + 1)) add(NE, 11);
		if (AstarNode* E  = get(x + 1, y    )) add(E,  8);
		if (AstarNode* SE = get(x + 1, y - 1)) add(SE, 11);
		if (AstarNode* S  = get(x    , y - 1)) add(S,  8);
		if (AstarNode* SW = get(x - 1, y - 1)) add(SW, 11);
		if (AstarNode* W  = get(x - 1, y    )) add(W,  8);
		if (AstarNode* NW = get(x - 1, y + 1)) add(NW, 11);

		if (open.empty())
			break;
		open.pop(node); // pop last element
	}
}


AstarNode* AstarGrid::get(int x, int y)
{
	if (x < 0 || Width <= x || y < 0 || Height <= y) return NULL; // world bounds checkin'
	return &Nodes[y * Width + x];
}
