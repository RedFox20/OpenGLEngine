/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#include "IShaderProgram.h"
#include "GL\glew.h"
#include "GL\glut.h"
#include <cstdio>
#include <malloc.h>

// prints the error log on this gl object
// assumes that an error occurrred
static void PrintError(unsigned obj)
{
	int log_length;
	if (glIsShader(obj))
		glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &log_length);
	else if (glIsProgram(obj))
		glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &log_length);
	else {
		fprintf(stderr, "Unexpected error during Shader compilation.\n");
		return;
	}

	log_length += 1;
	char* log = (char*)((log_length < 65536) ? alloca(log_length) : malloc(log_length));
	if (glIsShader(obj))
		glGetShaderInfoLog(obj, log_length, 0, log);
	else if (glIsProgram(obj))
		glGetProgramInfoLog(obj, log_length, 0, log);
	fprintf(stderr, "%s", log); // display the error
	if (log_length >= 65536) free(log); // free if necessary
}

// compiles string into a gl shader
static unsigned CompileShader(const char* sh_mem, int size, const char* idstr, GLenum type)
{
	unsigned shader = glCreateShader(type);
	glShaderSource(shader, 1, &sh_mem, &size);
	glCompileShader(shader);
	GLint compile_ok; glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_ok);
	if (!compile_ok)
	{
		fprintf(stderr, "%s: ", idstr);
		PrintError(shader);
		glDeleteShader(shader);
		return 0;
	}
	return shader; // yay succes.
}

// compiles a file into a shader
static unsigned CompileShaderFile(const char* sh_file, GLenum type)
{
	if (sh_file == NULL || !sh_file[0])
	{
		fprintf(stderr, "Error shader file '' is invalid!\n");
		return 0;
	}
	FILE* f = fopen(sh_file, "rb"); // readonly binary
	if (f == NULL)
	{
		fprintf(stderr, "Error shader file '%s' not found!\n", sh_file);
		return 0;
	}
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);
	char* buffer = (char*)(size < 65536 ? alloca(size+1) : malloc(size+1));
	unsigned shader = 0;
	if (fread(buffer, size, 1, f) == 1) // could we read in the data?
	{
		buffer[size] = '\0'; // since we actually read a string...
		shader = CompileShader(buffer, size, sh_file, type);
	}
	else fprintf(stderr, "Error shader file '%s' could not be read to the end!\n", sh_file);
	if (size >= 65536) free(buffer); // free if on the heap
	fclose(f);
	return shader;
}

static unsigned LinkShaderProgram(unsigned vs, unsigned fs)
{
	unsigned program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	GLint link_ok; glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok)
	{
		fprintf(stderr, "Linking failure! vs:%d fs:%d\n", vs, fs);
		PrintError(program);
	}
	return program;
}

bool IShaderProgram::CompileFromFile(const char* vs_file, const char* fs_file)
{
	if (program)
	{
		fprintf(stderr, "Shader program already compiled!\n");
		return false; // program already compiled!
	}
	unsigned vs = CompileShaderFile(vs_file, GL_VERTEX_SHADER);
	unsigned fs = CompileShaderFile(fs_file, GL_FRAGMENT_SHADER);
	if (vs && fs)
	{
		this->program = LinkShaderProgram(vs, fs);
		if (!LoadVariables()) // error loading any required variables?
			Destroy(); // destroy the shader program
	}
	if (vs) glDeleteShader(vs);
	if (fs) glDeleteShader(fs);
	return this->program ? true : false;
}

bool IShaderProgram::CompileFromString(const char* vs_str, const char* fs_str)
{
	if (program)
	{
		fprintf(stderr, "Shader program already compiled!\n");
		return false; // program already compiled
	}
	unsigned vs = CompileShader(vs_str, strlen(vs_str), ".vs", GL_VERTEX_SHADER);
	unsigned fs = CompileShader(fs_str, strlen(fs_str), ".fs", GL_FRAGMENT_SHADER);
	if (vs && fs)
	{
		this->program = LinkShaderProgram(vs, fs);
		if (!LoadVariables()) // error loading any required variables?
			Destroy(); // destroy the shader program
	}
	if (vs) glDeleteShader(vs);
	if (fs) glDeleteShader(fs);
	return this->program ? true : false;
}

void IShaderProgram::Destroy() // just GL delete the shader program
{
	if(program)
	{
		glDeleteProgram(program);
		program = 0;
	}
}





// currently bound program
const IShaderProgram* IShaderProgram::Program = NULL;


// binds this shader to the pipeline
void IShaderProgram::Bind()
{
	if (Program == this) return; // no need to change anything
	if (Program) Program->UnbindAttributes();
	Program = this;
	glUseProgram(program);
	BindAttributes();
}

// unbinds this shader from the pipeline
void IShaderProgram::Unbind()
{
	if (!Program) return; // don't trouble OpenGL with unnecessary null calls
	if (Program) Program->UnbindAttributes();
	Program = NULL;
	glUseProgram(0);
}

// binds this model-view-projection matrix to the shader
void IShaderProgram::BindMatrix(const glm::mat4& modelViewProjection)
{
	if (uTransform == -1) return; // no transform matrix available in shader
	glUniformMatrix4fv(uTransform, 1, GL_FALSE, (float*)&modelViewProjection);
}

// binds a raw opengl texture to this shader
void IShaderProgram::BindTexture(unsigned glTexture)
{
	if (uDiffuseTex == -1) return; // no diffuse texture available in shader
	if (!glTexture) return; // no material loaded?
	glBindTexture(GL_TEXTURE_2D, glTexture);
	glUniform1i(uDiffuseTex, 0); // texture 0
}

// binds a new texture to this shader
void IShaderProgram::BindTexture(const Texture* texture)
{
	if (uDiffuseTex == -1) return; // no diffuse texture available in shader
	if (!texture || !texture->glTexture) return; // no material loaded?
	glBindTexture(GL_TEXTURE_2D, texture->glTexture);
	glUniform1i(uDiffuseTex, 0); // texture 0
}

// binds a specific diffuse color to this shader
void IShaderProgram::BindDiffuseColor(const Vector4& colorRGBA)
{
	if (uDiffuseColor == -1) return; // no diffuse color available in shader
	glUniform4fv(uDiffuseColor, 1, (float*)&colorRGBA); 
}

// binds a specific background color to this shader
void IShaderProgram::BindOutlineColor(const Vector4& outlineRGBA)
{
	if (uOutlineColor == -1) return; // no diffuse color available in shader
	glUniform4fv(uOutlineColor, 1, (float*)&outlineRGBA); 
}

// draws a 3D model using this shader
void IShaderProgram::Draw(const VertexBuffer* vb)
{
	if (vb) vb->Draw();
}

// helper function for loading an attribute
bool IShaderProgram::Attribute(const char* attribute_name, int& out_attr)
{
	if ((out_attr = glGetAttribLocation(program, attribute_name)) != -1)
		return true;
	fprintf(stderr, "Failed to get attribute '%s' in the linked shader %s:%s\n", attribute_name, vsf, fsf);
	return false;
}

// helper function for loading a varying
bool IShaderProgram::Uniform(const char* uniform_name, int& out_uniform)
{
	if ((out_uniform = glGetUniformLocation(program, uniform_name)) != -1)
		return true;
	fprintf(stderr, "Failed to get uniform '%s' in the linked shader %s:%s\n", uniform_name, vsf, fsf);
	return false;
}




//// ------- MaterialShader implementation

void MaterialShader::BindAttributes() const 
{
	glEnableVertexAttribArray(aPosition);	// bind position
	glEnableVertexAttribArray(aCoord);		// bind coord
}

void MaterialShader::UnbindAttributes() const 
{
	glDisableVertexAttribArray(aPosition); // unbind position
	glDisableVertexAttribArray(aCoord); // unbind coord
}

bool MaterialShader::LoadVariables()
{
	return Attribute("position", aPosition)
		&& Attribute("coord", aCoord)
		&& Uniform("diffuseTex", uDiffuseTex) 
		&& Uniform("transform", uTransform);
}




//// ------- Shader2D implementation

void Shader2D::BindAttributes() const 
{
	glEnableVertexAttribArray(aVertex); // bind vertex attr
}

void Shader2D::UnbindAttributes() const 
{
	glDisableVertexAttribArray(aVertex); // unbind vertex attr
}

bool Shader2D::LoadVariables()
{
	// we require at least a diffuseTex or a diffuseColor
	// if both are missing, then return false for failure
	if (!(Uniform("diffuseTex", uDiffuseTex) | !Uniform("diffuseColor", uDiffuseColor)))
		return false;
	return Attribute("vertex", aVertex); // vertex is required though!
}




//// ------- TextShader2D implementation

bool TextShader2D::LoadVariables()
{
	Uniform("outlineColor", uOutlineColor); // not required
	return Shader2D::LoadVariables() 
		&& Uniform("transform", uTransform);
}





//// ------- ColorShader2D implementation

bool ColorShader2D::LoadVariables()
{
	return Attribute("vertex", aVertex)
		&& Uniform("transform", uTransform);
}

void ColorShader2D::BindAttributes() const 
{
	glEnableVertexAttribArray(aVertex); // bind vertex attr
	glEnableVertexAttribArray(1);
}

void ColorShader2D::UnbindAttributes() const 
{
	glDisableVertexAttribArray(aVertex); // unbind vertex attr
	glDisableVertexAttribArray(1);
}

