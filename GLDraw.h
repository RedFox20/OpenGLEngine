/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#pragma once
#ifndef GLDRAW_H
#define GLDRAW_H

#include "VertexBuffer.h"
#include <vector>
using std::vector;



/**
 * @note Packs an RGBA value into a single float
 * @param rgba An RGBA Vector4
 * @return Packed float representing the rgba value
 */
float PackRGBA(const Vector4& rgba);



/**
 * @note GLDraw is a helper object for creating 2D objects in OpenGL. 
 * @note The objects are generated as tri-polys and can be aliased or anti-aliased
 */
struct GLDraw
{
	vector<Vertex2ColorUnpacked> vertices; // buffer for vertices
	vector<Index> indices; // buffer for indices


	/**
	 * @note Creates a VertexIndexBuffer based on the current state
	 * @return A new VertexIndexBuffer
	 */
	VertexIndexBuffer* CreateBuffer() const;

	/**
	 * @note Clears the current drawing
	 */
	void Clear();

	/**
	 * @note Draws an anti-aliased line
	 * @param p1 Starting point of the line
	 * @param p2 Endinge point of the line
	 * @param color Color of the line
	 * @param width Width of the line
	 */
	void LineAA(const Vector2& p1, const Vector2& p2, const Vector4& color, float width = 1.0f);

	/**
	 * @note Creates a rectangle with AA lines
	 * @param origin Lower left corner of the rectangle - the origin
	 * @param size Width & Height of the rectangle
	 * @param color Color of the lines
	 * @param lineWidth[1.0f] Width of the lines to draw
	 */
	void RectAA(const Vector2& origin, const Vector2& size, const Vector4& color, float lineWidth = 1.0f);

	/**
	 * @note Creates a circle with AA lines
	 * @param center Center of the circle 
	 * @param radius Radius of the circle
	 * @param color Color of the lines
	 * @param lineWidth[1.0f] Width of the lines to draw
	 */
	void CircleAA(const Vector2& center, float radius, const Vector4& color, float lineWidth = 1.0f);

	/**
	 * @note Fills a rectangle
	 * @param origin Lower left corner of the rectangle - the origin
	 * @param size Width & Height of the rectangle
	 * @param color RGBA of the rectangle
	 */
	void FillRect(const Vector2& origin, const Vector2& size, const Vector4& color);
};



#endif // GLDRAW_H