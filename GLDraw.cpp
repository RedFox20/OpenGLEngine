/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#include "GLDraw.h"
#define _USE_MATH_DEFINES
#include <math.h>

	/**
	 * @note Packs an RGBA value into a single float
	 * @param rgba An RGBA Vector4
	 * @return Packed float representing the rgba value
	 */
	float PackRGBA(const Vector4& rgba)
	{
		return rgba.dot(Vector4(
			1.0f/(256.0f*256.0f*256.0),
			1.0f/(256.0f*256.0f),
			1.0f/(256.0f),
			1.0f)
		);
	}


	/**
	 * @note Creates a VertexIndexBuffer based on the current state
	 * @return A new VertexIndexBuffer
	 */
	VertexIndexBuffer* GLDraw::CreateBuffer() const
	{
		VertexIndexBuffer* vib = new VertexIndexBuffer();
		if(vertices.size())
		{
			vib->Create<Vertex2ColorUnpacked>(BUFFER_STATIC);
			vib->BufferVertices(&vertices[0], vertices.size());
			vib->BufferIndices(&indices[0], indices.size());
		}
		return vib;
	}


	/**
	 * @note Clears the current drawing
	 */
	void GLDraw::Clear()
	{
		vertices.clear();
		indices.clear();
	}


	/**
	 * @note Draws an anti-aliased line
	 * @param p1 Starting point of the line
	 * @param p2 Endinge point of the line
	 * @param color Color of the line
	 * @param width Width of the line
	 */
	void GLDraw::LineAA(const Vector2& p1, const Vector2& p2, const Vector4& color, float width)
	{
		// 12 vertices
		//      x1                A up     
		// 0\``2\``4\``6    left  |  right 
		// | \ | \ | \ |    <-----o----->  
		// 1__\3__\5__\7          |         
		//      x2                V down
		//float rgba = PackRGBA(color);
		//float rgbZ = PackRGBA(Vector4(color.x, color.y, color.z, 0.0f));
		Vector4 colorz(color.rgb, 0.0f);

		// core radius determines the width of the line core
		// for very small widths, the core should be very small ~10%
		// for large width, the core should be very large ~90%
		float cr; // core radius
		switch((int)width){
		case 0: case 1: width += 0.5f; cr = 0.25f; break;
		case 2: cr = 0.75f; break;
		case 3: cr = 1.5f; break;
		default: cr = (width / 2.0f) - 1.0f; break; // always leave 1 pixel for the edge radius
		}
		float w2 = width / 2.0f;
		float x1 = p1.x, y1 = p1.y, x2 = p2.x, y2 = p2.y;
		Vector2 right(y2 - y1, x1 - x2);
		right.normalize();
		float ex = right.x * w2, ey = right.y * w2; // edge xy offsets
		float cx = right.x * cr, cy = right.y * cr; // center xy offsets
		size_t n = vertices.size();
		vertices.resize(n + 8);
		Vertex2ColorUnpacked* v = &vertices[n];
		v[0].x = x1 - ex, v[0].y = y1 - ey, v[0].rgba = colorz;	// left-top
		v[1].x = x2 - ex, v[1].y = y2 - ey, v[1].rgba = colorz;	// left-bottom
		v[2].x = x1 - cx, v[2].y = y1 - cy, v[2].rgba = color;	// left-middle-top
		v[3].x = x2 - cx, v[3].y = y2 - cy, v[3].rgba = color;	// left-middle-bottom
		v[4].x = x1 + cx, v[4].y = y1 + cy, v[4].rgba = color;	// right-middle-top
		v[5].x = x2 + cx, v[5].y = y2 + cy, v[5].rgba = color;	// right-middle-bottom
		v[6].x = x1 + ex, v[6].y = y1 + ey, v[6].rgba = colorz;	// right-top
		v[7].x = x2 + ex, v[7].y = y2 + ey, v[7].rgba = colorz;	// right-bottom

		size_t numIndices = indices.size();
		indices.resize(numIndices + 18);
		Index* i = &indices[numIndices];
		i[0] = n + 0, i[1] = n + 1, i[2] = n + 3; // triangle 1
		i[3] = n + 0, i[4] = n + 3, i[5] = n + 2; // triangle 2
		i[6] = n + 2, i[7] = n + 3, i[8] = n + 5; // triangle 3
		i[9] = n + 2, i[10]= n + 5, i[11]= n + 4; // triangle 4
		i[12]= n + 4, i[13]= n + 5, i[14]= n + 7; // triangle 5
		i[15]= n + 4, i[16]= n + 7, i[17]= n + 6; // triangle 6
	}


	/**
	 * @note Creates a rectangle with AA lines
	 * @param origin Lower left corner of the rectangle - the origin
	 * @param size Width & Height of the rectangle
	 * @param color Color of the lines
	 * @param lineWidth[1.0f] Width of the lines to draw
	 */
	void GLDraw::RectAA(const Vector2& origin, const Vector2& size, const Vector4& color, float lineWidth)
	{
		//  0---3
		//  | + |
		//  1---2
		Vector2 p0(origin.x, origin.y + size.y);
		Vector2 p1(origin.x, origin.y);
		Vector2 p2(origin.x + size.x, origin.y);
		Vector2 p3(p2.x, p0.y);
		LineAA(p0, p1, color, lineWidth);
		LineAA(p1, p2, color, lineWidth);
		LineAA(p2, p3, color, lineWidth);
		LineAA(p3, p0, color, lineWidth);
	}


	/**
	 * @note Creates a circle with AA lines
	 * @param center Center of the circle 
	 * @param radius Radius of the circle
	 * @param color Color of the lines
	 * @param lineWidth[1.0f] Width of the lines to draw
	 */
	void GLDraw::CircleAA(const Vector2& center, float radius, const Vector4& color, float lineWidth)
	{
		// adaptive line count
		const int segments = 12 + (int(radius)/6);
		const float segmentArc = (2.0f * float(M_PI)) / segments;
		const float x = center.x, y = center.y;
		
		float alpha = segmentArc;
		Vector2 A(x, y + radius);
		for (int i = 0; i < segments; ++i, alpha += segmentArc)
		{
			Vector2 B(x + sinf(alpha)*radius, y + cosf(alpha)*radius);
			LineAA(A, B, color, lineWidth);
			A = B;
		}
	}


	/**
	 * @note Fills a rectangle
	 * @param origin Lower left corner of the rectangle - the origin
	 * @param size Width & Height of the rectangle
	 * @param color RGBA of the rectangle
	 */
	void GLDraw::FillRect(const Vector2& origin, const Vector2& size, const Vector4& color)
	{
		//  0---3   0\``3
		//  | + |   | \ |
		//  1---2   1--\2
		//float rgba = PackRGBA(color);
		float x1 = origin.x, x2 = origin.x + size.x;
		float y1 = origin.y, y2 = origin.y + size.y;
		size_t n = vertices.size();
		vertices.resize(n + 4);
		Vertex2ColorUnpacked* v = &vertices[n];
		v[0].x = x1, v[0].y = y2, v[0].rgba = color; // left-top
		v[1].x = x1, v[1].y = y1, v[1].rgba = color; // left-bottom
		v[2].x = x2, v[2].y = y1, v[2].rgba = color; // right-bottom
		v[3].x = x2, v[3].y = y2, v[3].rgba = color; // right-top

		//v[0].x = x1, v[0].y = y2, v[0].rgba = rgba; // left-top
		//v[1].x = x1, v[1].y = y1, v[1].rgba = rgba; // left-bottom
		//v[2].x = x2, v[2].y = y1, v[2].rgba = rgba; // right-bottom
		//v[3].x = x2, v[3].y = y2, v[3].rgba = rgba; // right-top
		size_t numIndices = indices.size();
		indices.resize(numIndices + 6);
		Index* i = &indices[numIndices];
		i[0] = n + 0, i[1] = n + 1, i[2] = n + 2;	// triangle 1
		i[3] = n + 0, i[4] = n + 2, i[5] = n + 3;	// triangle 2
	}



