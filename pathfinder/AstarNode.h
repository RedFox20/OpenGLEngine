#pragma once
#include <Basetypes.h>

struct AstarNode;

struct AstarLink
{
	AstarNode* node; // pointer to next node
	int gain;        // distance score gain if this link is traversed
};

struct AstarNode 
{
	int FScore;			// F = H(manhattan) + GScore score to destination
	int HScore;         // heuristic score - manhattan method
	int GScore;			// accumulated sum of G values g=(10 or 14
	uint OpenID;        // ID of an 'open' node, this is used to check if we've opened this node or not
	AstarNode* Prev;	// previous node in this path
	

	bool Closed;		// true if this node has been closed
	byte Plane;			// plane of this Node, collision plane is always 1
	byte _Dummy1;
	byte _Dummy2;

	int X, Y;			// the X, Y position in the world

	static const int MaxLinks = 8;
	int NumLinks;		// number of active links
	AstarLink Links[MaxLinks];
};
