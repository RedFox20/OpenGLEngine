/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#pragma once
#ifndef GUIOBJECT_H
#define GUIOBJECT_H

#include "VertexBuffer.h"
#include "Texture.h"
#include <gui/freetype.h>
#include "GLDraw.h"

enum GuiObjectType {
	GUIOBJECT = 1<<0,
	GUIGRAPHIC = 1<<1,
	GUITEXT = 1<<2,
};

// base GUI object with some basic
// position, rotation and scaling data
class GuiObject
{
protected:
	int type;
	Vector2 pos;					// position of the gui object xy[-1..1]
	Vector2 scl;					// scale of the gui object (default {1,1})
	float rot;						// rotation of the guiobject
	float z;						// depth component
	Vector2 size;					// size of this gui object

public:
	inline GuiObject() : type(GUIOBJECT), scl(1.0f, 1.0f), rot(0.0f) , z(0.0f), size(0.0f, 0.0f) {}

	inline const Vector2& Size() const { return size; }
	inline const Vector2& Pos() const { return pos; }
	inline const Vector2& Scale() const { return scl; }

	/** @return TRUE if this is an instance of the specified gui object type */
	bool InstanceOf(GuiObjectType t);

	// gets the object's rotate-scale-translation matrix
	void AffineTransform(Matrix4& out) const;

	// moves the object in the global coordinate space by the given offset
	void MoveGlobal(const Vector2& globalOffset);

	// moves the object in its local coordinate space by the given offset
	void MoveRelative(const Vector2& relOffset);

	// sets the position of the object, regardless of any rotation
	void SetPosition(const Vector2& pos);

	// sets the position of the object, regardless of any rotation
	void SetPosition(float x, float y);

	// sets the 2D scale of the object
	void SetScale(const Vector2& scale);

	// sets the 2D scale of the object
	void SetScale(float sx, float sy);

	// rotates the object around the z axis by given degrees
	void Rotate(float degrees);

	// sets the rotation of the object
	void SetRotation(float degrees);

	/** @brief Draws this guiobject in the context of the current shader */
	virtual void Draw(const Matrix4& viewProjection) const = 0;
};

// a gui overlay object
class GuiOverlay : public GuiObject
{
protected:
	VertexBuffer* vb;
public:
	GuiOverlay();
	~GuiOverlay();

	/** @brief Creates a new GuiOverlay object based on the current state of the GLDraw object */
	void Create(const GLDraw& draw);
	/** @brief Destroys the graphic object if needed */
	void Destroy();
	/** @brief Replaces existing GuiGraphic object based on the input GLDraw object */
	void Update(const GLDraw& draw);

	/** @brief Draws this GuiGraphic in the context of the current shader */
	void Draw(const Matrix4& viewProjection) const override;
};

// a gui text object
class GuiText : public GuiObject
{
protected:
	freetype::Text* text;
	Vector4 color;
	Vector4 outline;
public:

	GuiText();
	GuiText(freetype::Text* txt);
	GuiText(freetype::Font* font, const wchar_t* fmt, ...);
	~GuiText();

	inline freetype::Text* Text() const { return text; }
	inline freetype::Font* Font() const { return text->font; }

	/** @brief Creates a new text object */
	void Create(freetype::Font* font, const wchar_t* str);
	/** @brief Creates a new text object */
	void Create(freetype::Font* font, const wchar_t* str, size_t len);
	/** @brief Creates a new text object from a formatted string with args */
	void CreateF(freetype::Font* font, const wchar_t* fmt, ...);
	/** @brief Creates a new text object from a formatted string with args */
	void CreateV(freetype::Font* font, const wchar_t* fmt, va_list ap);

	/** @brief Sets the text and outline colors */
	void SetColor(const Vector4& textColor, const Vector4& outlineColor);

	/** @brief Destroys the text if needed */
	void Destroy();
	/** @brief Updates existing text object */
	void Update(const wchar_t* str, size_t len);
	/** @brief Inserts new text into this text object */
	void Insert(int index, const wchar_t* str, size_t len);
	/** @brief Appends new text to the end of this text object */
	void Append(const wchar_t* str, size_t len);

	/** @brief Draws this GuiText in the context of the current shader */
	void Draw(const Matrix4& viewProjection) const override;

};


#endif // GUIOBJECT_H