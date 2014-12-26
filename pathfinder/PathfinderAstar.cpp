#include "PathfinderAstar.h"


	void PathfinderAstar::Create(float cellSize, int width, int height, const byte* initData)
	{
		CellSize = cellSize;
		CellHalfSize = cellSize * 0.5f;
		OpenID = 0;
		Grid.create(width, height, initData);
	}
	void PathfinderAstar::Destroy()
	{
		Grid.destroy();
	}


	__forceinline static int abs_dist(int dist)
	{
		return dist >= 0 ? dist : -dist;
	}

	bool PathfinderAstar::Process(PfVector<Vector2>& outPath, PfVector<Vector2>* explored)
	{
		NumOpened   = 0;
		NumReopened = 0;
		AstarNode* head = Start;
		AstarNode* end  = End;

		if (head->Plane != end->Plane || head->Plane == 1 || end->Plane == 1)
			return false; // no possible path between these two, or collision planes(1)

		head->Prev = NULL;
		int openID = ++OpenID; // with 1000 * 60 pathfinds per second, this will overflow in: ~8.17 years
		int goalX  = end->X;
		int goalY  = end->Y;

		while (head != end)
		{
			AstarNode* prev  = head->Prev;
			AstarLink* link  = head->Links;
			AstarLink* elink = link + head->NumLinks;
			int headGScore   = head->GScore;
			byte planeID     = head->Plane;

			for (; link != elink; ++link)
			{
				AstarNode* n = link->node;
				if (n == prev || planeID != n->Plane)
					continue; // avoid circural references

				if (openID == n->OpenID) // we have opened this Node before
				{
					if (n->Closed) 
						continue; // don't touch it if it's CLOSED

					int gscore = headGScore + link->gain; // new gain score
					if (gscore >= n->GScore) 
						continue; // if the new gain is worse, then don't touch it

					n->FScore = n->HScore + gscore;
					n->GScore = gscore;
					n->Prev   = head;

					//// @note reopened:
					++NumOpened;
					++NumReopened;
					OpenList.repos(n); // reposition item
				}
				else
				{
					n->HScore = (abs_dist(goalX - n->X) + abs_dist(goalY - n->Y)) * 8; // (distX + distY)*8
					n->GScore = headGScore + link->gain;
					n->FScore = n->HScore + n->GScore;
					n->Closed = false;
					n->Prev   = head;
					n->OpenID = openID;

					//// @note first open:
					++NumOpened;
					OpenList.insert(n);
				}

				int size = OpenList.size();
				if (size > MaxDepth) MaxDepth = size;
				if (explored) explored->push_back(ToScreenCoordCentered(head)),
								explored->push_back(ToScreenCoordCentered(n));
			}

			// after inserting into the sorted list we get the heuristically best node available
			if (OpenList.empty())
				break;

			head = OpenList.pop();
			head->Closed = true;
		}

		// construct the out path
		head = end;
		do {
			outPath.push_back(ToScreenCoordCentered(head));
		} while(head = head->Prev);

		OpenList.clear(); // resets the pool
		return true;
	}






	
	Vector2 PathfinderAstar::ToScreenCoord(const AstarNode* node) const
	{
		return Vector2(node->X * CellSize, node->Y * CellSize);
	}
	Vector2 PathfinderAstar::ToScreenCoordCentered(const AstarNode* node) const
	{
		return Vector2(node->X * CellSize + CellHalfSize, node->Y * CellSize + CellHalfSize);
	}
	Vector2i PathfinderAstar::ToVirtualCoord(const Vector2& pos) const
	{
		return Vector2i(int(pos.x / CellSize), int(pos.y / CellSize));
	}
	Vector2i PathfinderAstar::ToVirtualCoord(float x, float y) const
	{
		return Vector2i(int(x / CellSize), int(y / CellSize));
	}


	bool PathfinderAstar::InWorld(const Vector2& pos) const
	{
		return 0.0f <= pos.x && pos.x < (Grid.Width * CellSize) 
			&& 0.0f <= pos.y && pos.y < (Grid.Width * CellSize);
	}
	bool PathfinderAstar::SetStart(const Vector2& worldXY) 
	{
		Vector2i pos = ToVirtualCoord(worldXY);
		return SetStart(pos.x, pos.y);
	}
	bool PathfinderAstar::SetStart(int x, int y)
	{
		AstarNode* n = Grid.get(x, y);
		if (n && n != Start && n != End) { // not null && not same && not same as end
			Start = n;
			n->FScore = 0;
			n->GScore = 0;
			return true;
		}
		return false;
	}

	bool PathfinderAstar::SetEnd(const Vector2& worldXY) 
	{
		Vector2i pos = ToVirtualCoord(worldXY);
		return SetEnd(pos.x, pos.y);
	}
	bool PathfinderAstar::SetEnd(int x, int y)
	{
		AstarNode* n = Grid.get(x, y);
		if(n && n != End && n != Start) { // not null && not same && not same as start
			End = n;
			n->FScore = 0;
			n->GScore = 0;
			return true;
		}
		return false;
	}
