/**
 * Copyright (c) 2013 - Jorma Rebane
 */

#include "GameObject.h"
#include "GL\glm\gtx\quaternion.hpp" // toMat4
#include "GL\glm\gtx\transform.hpp" // scale, translate

	// gets the object's rotate-scale-translation matrix
	void GameObject::AffineTransform(Matrix4& out) const
	{
		glm::mat4 R = glm::toMat4(rot); // right-most matrix can't be a temporary object
		glm::mat4 S = glm::scale<float>(scl);
		glm::mat4 P = glm::translate<float>(pos);
		out = P * S * R; // affine transform
	}

	// moves the object in the global coordinate space by the given offset
	void GameObject::MoveGlobal(const Vector3& globalOffset)
	{
		pos += globalOffset;
	}

	// moves the object in its local coordinate space by the given offset
	void GameObject::MoveRelative(const Vector3& relOffset)
	{
		pos = pos + (rot * (glm::vec3&)relOffset);
	}

	// sets the position of the object, regardless of any rotation
	void GameObject::SetPosition(const Vector3& pos)
	{
		this->pos = pos;
	}

	// rotates the object around the given axis by the given degrees
	void GameObject::Rotate(const Vector3& axis, float degrees)
	{
		rot = glm::angleAxis<float>(degrees, axis) * rot;
	}


	/**
	 * Draws this object using the specified shader object
	 * @param shader Shader program to draw this object with
	 * @param viewProjection A Matrix4[Projection * View] to translate the object pos
	 */
	void GameObject::Draw(const Matrix4& viewProjection) const
	{
		ShaderProgram* shader = ShaderProgram::CurrentShader();

		Matrix4 transform; 
		AffineTransform(transform);
		shader->BindMatrix(viewProjection * transform); // upload matrix to the shader
		shader->BindTexture(texture);
		shader->Draw(vbuffer);							// draw the vbuffer
	}