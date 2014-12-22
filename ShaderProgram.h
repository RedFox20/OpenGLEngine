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
#include <gui/freetype.h>
#include "GL\glm\glm.hpp"
#include <utils/io.h>


/**
 * @brief Generic flexib ShaderProgram
 */
struct ShaderProgram
{
private:
	static const ShaderProgram* Program; // currently bound program
public:
	/** @return The currently bound shader object */
	static inline ShaderProgram* CurrentShader() { return (ShaderProgram*)Program; }

protected:


	int program;	     // open GL shader program
	std::string vsf;     // vertex shader file
	std::string fsf;     // fragment shader file
	time_t vsmod;        // last modified time of vertex shader file
	time_t fsmod;        // last modified time of fragment shader file
	dirwatch shaderdir;  // directory monitor for hotloading

	char uniforms[u_MaxUniforms];     // active uniforms
	char attributes[a_MaxAttributes]; // active attributes


public:
	/**
	 * @brief Creates an uninitialized ShaderProgram
	 */
	ShaderProgram();
	/**
	 * @brief Creates a new shader program linked to the specified files
	 * @note  The files are not loaded before Compile() is called
	 */
	ShaderProgram(const char* vs_file, const char* fs_file);
	/**
	 * @brief Creates a new shader program linked to the specified files
	 * @note  The files are not loaded before Compile() is called
	 */
	ShaderProgram(const std::string& vs_file, const std::string& fs_file);
	~ShaderProgram();

	/**
	 * @brief Compiles the shader program using the constructor initialized file names
	 */
	bool Compile();

	/** 
	 * @brief Compiles the shader program from the specified files.
	 * @param vs_file Path to vertex shader
	 * @param fs_file Path to fragment shader
	 */
	bool CompileFromFile(const char* vs_file, const char* fs_file);

	/**
	 * @brief Compiles the shader program from the specified strings.
	 * @param vs_str Vertex shader string
	 * @param fs_str Fragment shader string
	 */
	bool CompileFromString(const char* vs_str, const char* fs_str);

	/** @brief destroys all resources */
	void Destroy();

	/** @brief Performs hotload if Vertex or Fragment programs changed */
	void HotLoad();

	/** @return TRUE if the shader is initialized */
	inline operator bool() const { return !!program; }

	/** @return TRUE if the shader is initialized */
	inline bool IsCreated() const { return !!program; }

	/** @return Specified uniform value or -1 if uniform not active */
	inline int Uniform(ShaderUniform uniform_id)
	{
		return (int)uniforms[uniform_id];
	}

	/** @return Specfied attribute value or -1 if attribute not active */
	inline int Attribute(ShaderAttribute attribute_id)
	{
		return (int)attributes[attribute_id];
	}

	/**
	 * @brief Binds this shader to the pipeline
	 * @note Shader is automatically Unbinded when another shader is bound!
	 * @note If this shader is already bound, no state is changed (which is good)!
	 */
	void Bind();

	/** @brief Unbinds this shader from the pipeline */
	void Unbind();

	/** @brief Binds this model-view-projection matrix to the shader u_Transform slot */
	void BindMatrix(const glm::mat4& modelViewProjection);

	/** @brief Binds a raw opengl texture to this shader u_DiffuseTex slot */
	void BindTexture(unsigned glTexture);

	/** @brief Binds a texture object to this shader u_DiffuseTex slot */
	void BindTexture(const Texture* texture);

	/** @brief Binds a specific diffuse color to this shader u_DiffuseColor slot */
	void BindDiffuseColor(const Vector4& colorRGBA);

	/** @brief Binds a specific background color to this shader u_OutlineColor slot */
	void BindOutlineColor(const Vector4& outlineRGBA);

	/** @brief Draws a VertexBuffer through this shader */
	void Draw(const VertexBuffer* vb);

protected:

	// load the shader specific variables
	void LoadVariables();

	// bind shader specific attributes
	void BindAttributes() const;

	// unbind shader specific attributes
	void UnbindAttributes() const;

};


#endif // SHADER_PROGRAM_H