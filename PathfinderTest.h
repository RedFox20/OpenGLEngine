/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#pragma once
#ifndef PATHFINDER_TEST_H
#define PATHFINDER_TEST_H

#include "IShaderProgram.h"

extern Vector2 gScreen; // current screen size

struct PathfinderTest
{
	static void Create();
	static void Destroy();
	static void DrawScene(
		TextShader2D* texts, 
		ColorShader2D* gui, 
		const Matrix4& projection);
};


#endif // PATHFINDER_TEST_H