#include "DistanceField.h"
#include <math.h>

struct Point
{
	int dx, dy, dist_sq;
};

static Point inside = { 0, 0, 0 };
static Point empty  = { 9999, 9999, (9999*9999 + 9999*9999) };

struct Grid
{
	int width, height;
	Point* data; // [width][height]

	inline Grid(int width, int height) : width(width), height(height), data(new Point[width*height])
	{
	}
	inline ~Grid()
	{
		delete[] data;
	}
	inline Point& operator[](int index)
	{
		if (index >= (width*height))
		{
			throw "Index out of range on DistanceField::Grid.";
		}
		return data[index];
	}
	inline void put(int x, int y, const Point& p)
	{
		data[y*width + x] = p;
	}
	inline const Point& get(int x, int y) const
	{
		return data[y*width + x];
	}
	inline const Point& get_checked(int x, int y) const
	{
		if (x >= 0 && y >= 0 && x < width && y < height)
			return data[y*width + x];
		return empty;
	}
	inline void compare(Point& p, int x, int y, int offsetx, int offsety)
	{
		Point other = get_checked(x + offsetx, y + offsety);
		other.dx += offsetx;
		other.dy += offsety;
		other.dist_sq = other.dx*other.dx + other.dy*other.dy;
		if (other.dist_sq < p.dist_sq)
			p = other;

		//int newX = x + offsetx, newY = y + offsety;
		//if (0 <= newX && 0 <= newY && newX < width && newY < height)
		//{
		//	Point newp = data[newX*width + newY];
		//	newp.dx += offsetx;
		//	newp.dy += offsety;
		//	newp.dist_sq = newp.dx*newp.dx + newp.dy*newp.dy;

		//	if (newp.dist_sq < p.dist_sq)
		//		p = newp;
		//}
	}
};

static void generate_sdf_grid(Grid& grid)
{
	int width = grid.width, height = grid.height;
	// Pass 0
	for (int y = 0, xy = 0; y < height; ++y, xy += width)
	{
		for (int x = 0; x < width; ++x) // forward on X axis
		{
			Point p = grid[xy];
			grid.compare(p, x, y, -1, 0);
			grid.compare(p, x, y, 0, -1);
			grid.compare(p, x, y, -1, -1);
			grid.compare(p, x, y, 1, -1);
			grid[xy++] = p;
		}

		for (int x = width - 1; x >= 0; --x) // backwars again on X axis
		{
			Point p = grid[--xy];
			grid.compare(p, x, y, 1, 0);
			grid[xy] = p;
		}
	}

	// Pass 1
	for (int y = height - 1, xy = (y+1)*width; y >= 0; --y, xy -= width)
	{
		for (int x = width - 1; x >= 0; --x) // backwards on X axis
		{
			Point p = grid[--xy];
			grid.compare(p, x, y, 1, 0);
			grid.compare(p, x, y, 0, 1);
			grid.compare(p, x, y, -1, 1);
			grid.compare(p, x, y, 1, 1);
			grid[xy] = p;
		}

		for (int x = 0; x < width; ++x) // forward again on X axis
		{
			Point p = grid[xy];
			grid.compare(p, x, y, -1, 0);
			grid[xy++] = p;
		}
	}
}

void convert_to_sdf(int width, int height, unsigned char* data, float radius)
{
	Grid grid1(width, height); // Points inside
	Grid grid2(width, height); // Points outside

	for (int y = 0, xy = 0; y < height; ++y) // for each row
	{
		for (int x = 0; x < width; ++x, ++xy) // for each pixel in row
		{
			register Point* p1, *p2;
			if (data[xy] < 128)
				p1 = &inside, p2 = &empty;
			else
				p1 = &empty, p2 = &inside;

			// Points inside get marked with a dx/dy of zero.
			// Points outside get marked with an infinitely large distance.
			grid1[xy] = *p1;
			grid2[xy] = *p2;
		}
	}

	generate_sdf_grid(grid1);
	generate_sdf_grid(grid2);

	for (int y = 0, xy = 0; y < height; ++y) // for each row
	{
		for (int x = 0; x < width; ++x, ++xy) // for each pixel in row
		{
			// Calculate the actual distance from the dx/dy
			float dist1 = sqrtf((float) grid1[xy].dist_sq );
			float dist2 = sqrtf((float) grid2[xy].dist_sq );

			//float dist = dist1 - dist2;
			//dist = dist * 3 + 128.0f;
			//if (dist > 255.0f) dist = 255.0f;
			//else if (dist < 0.0f) dist = 0.0f;
			//data[xy] = (unsigned char)(dist);

			float dist = (dist1 - dist2) / radius;
			dist = (dist * 0.5f) + 0.5f;

			if (dist > 1.0f)      dist = 1.0f;
			else if (dist < 0.0f) dist = 0.0f;
			data[xy] = (unsigned char)(dist * 255.0f);
		}
	}

}