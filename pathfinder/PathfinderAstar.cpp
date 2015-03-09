#include "PathfinderAstar.h"


	void PathfinderAstar::Create(float cellSize, int width, int height, const byte* initData)
	{
		CellSize = cellSize;
		CellHalfSize = cellSize * 0.5f;
		OpenID = 0;
		Grid.create(width, height, initData);
		OpenList.reserve((width + height) * 4);
	}
	void PathfinderAstar::Destroy()
	{
		Grid.destroy();
	}


	/**
	 * @warning This function is heavily optimized using profile guided optimization hints
	 *          and cache stall info. If you plan to optimize/change this function, please 
	 *          use a profiler to measure changes
	 */
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

			for (; link != elink; ++link)
			{
				AstarNode* n = link->node;
				if (n == prev || n->Plane == 1)
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
					// calculate HScore
					// (abs(distX) + abs(distY))*8
					int HScore = goalX - n->X, diffY = goalY - n->Y;
					if (HScore < 0) HScore = -HScore;
					if (diffY  < 0) diffY = -diffY;
					HScore += diffY;
					HScore <<= 3;

					int GScore = headGScore + link->gain;
					n->HScore = HScore;
					n->GScore = GScore;
					n->FScore = HScore + GScore;
					n->Closed = false;
					n->Prev   = head;
					n->OpenID = openID;

					//// @note first open:
					++NumOpened;
					OpenList.insert(n);
				}

				int size = OpenList.size();
				if (size > MaxDepth) MaxDepth = size;

				if (explored)
				{
					explored->push_back(ToScreenCoordCentered(head));
					explored->push_back(ToScreenCoordCentered(n));
				}
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
			Vector2 coord = ToScreenCoordCentered(head);
			outPath.push_back(coord);
		} while (head = head->Prev);

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
		return ToVirtualCoord(pos.x, pos.y);
	}
	Vector2i PathfinderAstar::ToVirtualCoord(float x, float y) const
	{
		//float cx = x - CellHalfSize, cy = y - CellHalfSize;
		float cx = x, cy = y;
		if (!InWorld(cx, cy))
			return Vector2i(-1, -1);
		return Vector2i(int(cx / CellSize), int(cy / CellSize));
	}


	bool PathfinderAstar::InWorld(float x, float y) const
	{
		return 0.0f <= x && x < (Grid.Width  * CellSize) 
			&& 0.0f <= y && y < (Grid.Height * CellSize);
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
