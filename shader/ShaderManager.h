/**
 * Copyright (c) 2015 - Jorma Rebane
 */
#pragma once
#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H
#include "ShaderProgram.h"
#include <vector>


/**
 * @brief Collection of shader defines
 */
struct ShaderDefines
{
	struct Def
	{
		int defined; // 1 if defined, 0 if disabled
		int value;   // the defined value
	};
	Def Defines[ShaderDefine::d_MaxShaderDefines];

	ShaderDefines();
	/** @brief Add or set a definition value */
	void Set(ShaderDefine key, int value);
	/** @brief Remove this definition */
	void Remove(ShaderDefine key);
	bool IsDefined(ShaderDefine key) const;
	int  GetValue(ShaderDefine key)  const;
	const Def& operator[](ShaderDefine key) const;
};


struct ShaderManager
{



	static ShaderProgram* GetShader(const char* shaderFile);

	/**
	 * @brief Unloads all cached shader files
	 */
	static void UnloadShaders();
};


#endif