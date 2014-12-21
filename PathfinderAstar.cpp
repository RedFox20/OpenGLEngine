#include "PathfinderAstar.h"


int AstarGrid::FillPlanes(int count, int firstPlane)
{
	PfVector<AstarNode*> open;
	for(int i = 0; i < count; i++)
	{
		if(!Nodes[i].Plane) 
			QuickFill(open, &Nodes[i], firstPlane++);
	}
	open.clear();
	return firstPlane;
}


void AstarGrid::QuickFill(PfVector<AstarNode*>& open, AstarNode* node, int plane)
{
	// this is actually similar to the A* algo
	while(true)
	{
		int x = node->X, y = node->Y;
		AstarNode* N  = Get(x    , y + 1); // N
		AstarNode* NE = Get(x + 1, y + 1); // NE
		AstarNode* E  = Get(x + 1, y    ); // E
		AstarNode* SE = Get(x + 1, y - 1); // SE
		AstarNode* S  = Get(x    , y - 1); // S
		AstarNode* SW = Get(x - 1, y - 1); // SW
		AstarNode* W  = Get(x - 1, y    ); // W
		AstarNode* NW = Get(x - 1, y + 1); // NW

		if(N && !N->Plane)		N->Plane = plane,	open.push_back(N);
		if(NE && !NE->Plane)	NE->Plane = plane,	open.push_back(NE);
		if(E && !E->Plane)		E->Plane = plane,	open.push_back(E);
		if(SE && !SE->Plane)	SE->Plane = plane,	open.push_back(SE);
		if(S && !S->Plane)		S->Plane = plane,	open.push_back(S);
		if(SW && !SW->Plane)	SW->Plane = plane,	open.push_back(SW);
		if(W && !W->Plane)		W->Plane = plane,	open.push_back(W);
		if(NW && !NW->Plane)	NW->Plane = plane,	open.push_back(NW);

		if(open.empty())
			break;
		open.pop_back(node);
	}
}



	static dynamic_pool<no_gc> QTreeAlloc(sizeof(AstarQNode), 1024);

	AstarQNode* AstarQNode::NewNode()
	{
		return (AstarQNode*)QTreeAlloc.alloc();
	}
	void AstarQNode::DeleteNode(AstarQNode* n)
	{
		QTreeAlloc.dealloc(n);
	}