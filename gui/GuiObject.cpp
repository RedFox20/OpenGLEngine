/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#include "GuiObject.h"
#include "GL\glm\gtx\transform.hpp" // scale, translate
#include "GL\glm\gtx\rotate_vector.hpp" // rotate 2d vector
#include <stdarg.h>

	/** @return TRUE if this is an instance of the specified gui object type */
	bool GuiObject::InstanceOf(GuiObjectType t)
	{
		return type & t ? true : false;
	}

	// gets the object's rotate-scale-translation matrix
	void GuiObject::AffineTransform(Matrix4& out) const
	{
		glm::mat4 R = glm::rotate(rot, 0.0f, 0.0f, 1.0f); // rotate around z axis
		glm::mat4 S = glm::scale(scl.x, scl.y, 1.0f); // no scaling in z axis
		glm::mat4 P = glm::translate(pos.x, pos.y, z); // add the z depth component
		out = P * R; // affine transform
	}

	// moves the object in the global coordinate space by the given offset
	void GuiObject::MoveGlobal(const Vector2& globalOffset)
	{
		pos += globalOffset;
	}

	// moves the object in its local coordinate space by the given offset
	void GuiObject::MoveRelative(const Vector2& relOffset)
	{
		pos += glm::rotate<float>(relOffset, rot);
	}

	// sets the position of the object, regardless of any rotation
	void GuiObject::SetPosition(const Vector2& newPos)
	{
		pos.x = roundf(newPos.x), pos.y = roundf(newPos.y);
	}

	// sets the position of the object, regardless of any rotation
	void GuiObject::SetPosition(float x, float y)
	{
		pos.x = roundf(x), pos.y = roundf(y);
	}

	// sets the 2D scale of the object
	void GuiObject::SetScale(const Vector2& scale)
	{
		scl = scale;
	}

	// sets the 2D scale of the object
	void GuiObject::SetScale(float sx, float sy)
	{
		scl.x = sx, scl.y = sy;
	}

	// rotates the object around the z axis by the given degrees
	void GuiObject::Rotate(float degrees)
	{
		rot += degrees;
	}

	// sets the rotation of the object
	void GuiObject::SetRotation(float degrees)
	{
		rot = degrees;
	}



	
	GuiOverlay::GuiOverlay() : vb(NULL)
	{
		type |= GUIGRAPHIC;
	}

	GuiOverlay::~GuiOverlay()
	{
		Destroy();
	}

	void GuiOverlay::Create(const GLDraw& draw)
	{
		if (vb) delete vb;
		vb = draw.CreateBuffer();
	}
	void GuiOverlay::Destroy()
	{
		if (vb) delete vb, vb = NULL;
	}
	void GuiOverlay::Update(const GLDraw& draw)
	{
		if (vb) delete vb;
		vb = draw.CreateBuffer();
	}

	void GuiOverlay::Draw(const Matrix4& viewProjection) const
	{
		if (vb && vb->IsCreated())
		{
			ShaderProgram* shader = ShaderProgram::CurrentShader();
			Matrix4 transform; AffineTransform(transform);
			shader->BindMatrix(viewProjection * transform); // upload matrix to the shader
			//shader->BindTexture(texture);
			vb->Draw();
		}
	}




	
	GuiText::GuiText() : text(NULL),		// no text yet
		color(1.0f, 1.0f, 1.0f, 1.0f),		// white text
		outline(0.05f, 0.05f, 0.05f, 1.0f)	// very dark grey shadow
	{
		type |= GUITEXT;
	}

	GuiText::GuiText(freetype::Text* txt) : text(txt),
		color(1.0f, 1.0f, 1.0f, 1.0f),      // white text
		outline(0.05f, 0.05f, 0.05f, 1.0f)  // very dark grey shadow
	{
		type |= GUITEXT;
	}
	GuiText::GuiText(freetype::Font* font, const wchar_t* fmt, ...) : text(NULL),
		color(1.0f, 1.0f, 1.0f, 1.0f),      // white text
		outline(0.05f, 0.05f, 0.05f, 1.0f)  // very dark grey shadow
	{
		type |= GUITEXT;
		va_list ap; va_start(ap, fmt);
		CreateV(font, fmt, ap);
	}

	GuiText::~GuiText()
	{
		Destroy();
	}

	void GuiText::Create(freetype::Font* font, const wchar_t* str)
	{
		Create(font, str, wcslen(str));
	}

	void GuiText::Create(freetype::Font* font, const wchar_t* str, size_t len)
	{
		if (text) delete text;
		text = font->CreateText(str, len);
		size = text->Size();
	}

	void GuiText::CreateF(freetype::Font* font, const wchar_t* fmt, ...)
	{
		va_list ap; va_start(ap, fmt);
		if (text) delete text;
		text = font->CreateTextV(fmt, ap);
		size = text->Size();
	}
	void GuiText::CreateV(freetype::Font* font, const wchar_t* fmt, va_list ap)
	{
		if (text) delete text;
		text = font->CreateTextV(fmt, ap);
		size = text->Size();
	}

	void GuiText::SetColor(const Vector4& textColor, const Vector4& outlineColor)
	{
		color = textColor;
		outline = outlineColor;
	}

	void GuiText::Destroy()
	{
		if (text) delete text, text = NULL;
	}
	void GuiText::Update(const wchar_t* str, size_t len)
	{
		if (text) text->Create(str, len), size = text->Size();
	}
	void GuiText::Insert(int index, const wchar_t* str, size_t len)
	{
		if (text) text->Insert(index, str, len), size = text->Size();
	}
	void GuiText::Append(const wchar_t* str, size_t len)
	{
		if (text) text->Append(str, len), size = text->Size();
	}


	void GuiText::Draw(const Matrix4& viewProjection) const
	{
		if (text && text->IsCreated()) {
			Matrix4 transform; AffineTransform(transform);
			text->Draw(viewProjection * transform, color, outline);
		}
	}