/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#include "Vector234.h"

const Vector2 Vector2::ZERO	(+0.f, +0.f); //
const Vector2 Vector2::UP	(+0.f, +1.f); // +Y
const Vector2 Vector2::DOWN	(+0.f, -1.f); // -Y
const Vector2 Vector2::LEFT	(-1.f, +0.f); // -X
const Vector2 Vector2::RIGHT(+1.f, +0.f); // +X
const Vector2 Vector2::UPLEFT	 = Vector2(-1.f, +1.f).normalized(); // -X +Y
const Vector2 Vector2::UPRIGHT	 = Vector2(+1.f, +1.f).normalized(); // +X +Y
const Vector2 Vector2::DOWNLEFT	 = Vector2(-1.f, -1.f).normalized(); // -X -Y
const Vector2 Vector2::DOWNRIGHT = Vector2(+1.f, -1.f).normalized(); // +X -Y


const Vector2i Vector2i::ZERO	(+0, +0); //
const Vector2i Vector2i::UP		(+0, +1); // +Y
const Vector2i Vector2i::DOWN	(+0, -1); // -Y
const Vector2i Vector2i::LEFT	(-1, +0); // -X
const Vector2i Vector2i::RIGHT	(+1, +0); // +X
const Vector2i Vector2i::UPLEFT    = Vector2i(-1, +1).normalized(); // -X +Y
const Vector2i Vector2i::UPRIGHT   = Vector2i(+1, +1).normalized(); // +X +Y
const Vector2i Vector2i::DOWNLEFT  = Vector2i(-1, -1).normalized(); // -X -Y
const Vector2i Vector2i::DOWNRIGHT = Vector2i(+1, -1).normalized(); // +X -Y