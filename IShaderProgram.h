/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#pragma once
#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <string>

#include "VertexBuffer.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include "FreeType.h"
#include "GL\glm\glm.hpp"



struct IShaderProgram
{
private:
	static const IShaderProgram* Program; // currently bound program
public:
	/** @return The currently bound shader object */
	static inline IShaderProgram* CurrentShader() { return (IShaderProgram*)Program; }

protected:
	unsigned program;	// open GL shader program
	const char* vsf;	// vertex shader file
	const char* fsf;	// fragment shader file
	int uTransform;		// model-view-projection matrix uniform location
	int uDiffuseTex;	// diffuse texture uniform
	int uDiffuseColor;	// diffuse color uniform
	int uOutlineColor;	// background color uniform
public:
	// auto-destroy the object
	inline ~IShaderProgram() { Destroy(); }

	// compiles the shader program using the constructor
	// initialized file names
	inline bool Compile() { return CompileFromFile(vsf, fsf); }

	// Compiles the shader program from the specified files.
	// @param vs_file Path to vertex shader
	// @param fs_file Path to fragment shader
	bool CompileFromFile(const char* vs_file, const char* fs_file);

	// Compiles the shader program from the specified strings.
	// @param vs_str Vertex shader string
	// @param fs_str Fragment shader string
	bool CompileFromString(const char* vs_str, const char* fs_str);

	// destroys all resources
	void Destroy();

	// is the shader initialized?
	inline operator bool() const { return program ? true : false; }

	/** @return TRUE if the shader is initialized */
	inline bool IsCreated() const { return program ? true : false; }




	// binds this shader to the pipeline
	// @note Shader is automatically Unbinded when another shader is bound!
	// @note2 If this shader is already bound, no state is changed (which is good)!
	void Bind();

	// unbinds this shader from the pipeline
	void Unbind();

	// binds this model-view-projection matrix to the shader
	void BindMatrix(const glm::mat4& modelViewProjection);

	// binds a raw opengl texture to this shader
	void BindTexture(unsigned glTexture);

	// binds a texture object to this shader
	void BindTexture(const Texture* texture);

	// binds a specific diffuse color to this shader
	void BindDiffuseColor(const Vector4& colorRGBA);

	// binds a specific background color to this shader
	void BindOutlineColor(const Vector4& outlineRGBA);

	// draws a VertexBuffer
	void Draw(const VertexBuffer* vb);


protected:
	// Creates an uninitialized ShaderProgram
	inline IShaderProgram() 
		: program(0), vsf(0), fsf(0), uTransform(-1), uDiffuseTex(-1), uDiffuseColor(-1), uOutlineColor(-1) {}

	// creates a new shader program linked to the specified files
	// the files are not yet loaded
	inline IShaderProgram(const char* vs_file, const char* fs_file) 
		: program(0), vsf(vs_file), fsf(fs_file), uTransform(-1), uDiffuseTex(-1), uDiffuseColor(-1), uOutlineColor(-1) {}

	// helper function for attribute loading
	bool Attribute(const char* attribute_name, int& out_attribute);

	// helper function for uniform loading
	bool Uniform(const char* uniform_name, int& out_uniform);


	// bind shader specific attributes
	virtual void BindAttributes() const = 0;

	// unbind shader specific attributes
	virtual void UnbindAttributes() const  = 0;

	// load the shader specific variables
	virtual bool LoadVariables() = 0;
};



struct MaterialShader : public IShaderProgram
{
	// position attribute
	int aPosition;
	// normal attribute
	int aNormal;
	// tex coord attribute
	int aCoord;

	// default initializes a MaterialShader
	inline MaterialShader() : aPosition(-1), aNormal(-1), aCoord(-1) {}

	// initializes a new MaterialShader prepared to load the specified shader files:
	inline MaterialShader(const char* vs_file, const char* fs_file) 
		: IShaderProgram(vs_file, fs_file), aPosition(-1), aNormal(-1), aCoord(-1) {}

protected:
	// bind materialshader and its attributes
	virtual void BindAttributes() const override;

	// unbinds materialshader and its attributes
	virtual void UnbindAttributes() const override;

	// loads MaterialShader variables
	virtual bool LoadVariables() override;
};



/**
 * A generic 2D shader
 */
struct Shader2D : public IShaderProgram
{
	// 2d vertex attribute vec4(pos.xy, tex.st)
	int aVertex;
	
	// default initialize
	inline Shader2D() : aVertex(-1) {}

	// initializes a new Shader2D prepared to load the specified shader files:
	inline Shader2D(const char* vs_file, const char* fs_file)
		: IShaderProgram(vs_file, fs_file), aVertex(-1) {}

protected:
	// bind materialshader and its attributes
	virtual void BindAttributes() const override;

	// unbinds materialshader and its attributes
	virtual void UnbindAttributes() const override;

	// loads MaterialShader variables
	virtual bool LoadVariables() override;
};



/**
 * A specialized gui text shader
 */
struct TextShader2D : public Shader2D
{
	// default initialize
	inline TextShader2D(){}

	// initializes a new GuiShader prepared to load the specified shader files:
	inline TextShader2D(const char* vs_file, const char* fs_file)
		: Shader2D(vs_file, fs_file) {}

protected:
	// loads MaterialShader variables
	virtual bool LoadVariables() override;
};


/**
 * A specialized gui color shader
 */
struct ColorShader2D : public Shader2D
{
	// default initialize
	inline ColorShader2D() {}

	// initializes a new GuiShader prepared to load the specified shader files:
	inline ColorShader2D(const char* vs_file, const char* fs_file)
		: Shader2D(vs_file, fs_file) {}

protected:
	// loads MaterialShader variables
	virtual bool LoadVariables() override;

	// bind materialshader and its attributes
	virtual void BindAttributes() const override;

	// unbinds materialshader and its attributes
	virtual void UnbindAttributes() const override;
};



#endif // SHADER_PROGRAM_H