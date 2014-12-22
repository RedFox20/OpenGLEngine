/**
 * Copyright (c) 2013 - Jorma Rebane
 * A generic GameObject interface - just to make things simple
 */
#pragma once
#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "ShaderProgram.h"

class GameObject
{
protected:
	VertexBuffer* vbuffer;
	const Texture* texture;
	Quaternion rot; // quaternion rotation
	Vector3 pos; // current position
	Vector3 scl; // current scale

public:
	// default initializes the transformation attributes
	inline GameObject() : vbuffer(0), texture(0), scl(1.0f, 1.0f, 1.0f) {}


	// initializes this object - user implemented
	virtual void Create() = 0;

	// destroys this object - user implemented
	virtual void Destroy() = 0;

	// gets the bound model
	inline const VertexBuffer* GetVertexBuffer() const { return vbuffer; }

	// gets the bound texture
	inline const Texture* GetTexture() const { return texture; }

	// sets the bound texture
	inline void SetTexture(const Texture* tex) { texture = tex; }



	// gets the object's rotate-scale-translation matrix
	void AffineTransform(Matrix4& out) const;

	// moves the object in the global coordinate space by the given offset
	void MoveGlobal(const Vector3& globalOffset);

	// moves the object in its local coordinate space by the given offset
	void MoveRelative(const Vector3& relOffset);

	// sets the position of the object, regardless of any rotation
	void SetPosition(const Vector3& pos);

	// rotates the object around the given axis by the given degrees
	void Rotate(const Vector3& axis, float degrees);


	/**
	 * Draws this object using the specified shader object
	 * @param shader Shader program to draw this object with
	 * @param viewProjection A Matrix4[Projection * View] to translate the object pos
	 */
	virtual void Draw(const Matrix4& viewProjection) const;
};




#endif // GAME_OBJECT_H