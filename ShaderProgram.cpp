/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#include "ShaderProgram.h"
#include "GL\glew.h"
#include "GL\glut.h"
#include <malloc.h>



static const char* UniformMap[] = {
	"transform",     // u_Transform
	"diffuseTex",    // u_DiffuseTex
	"specularTex",   // u_SpecularTex
	"normalTex",     // u_NormalTex
	"shadowTex",     // u_ShadowTex
	"occludeTex",    // u_OccludeTex
	"diffuseColor",  // u_DiffuseColor
	"outlineColor",  // u_OutlineColor
};
static const char* AttributeMap[] = {
	"position",      // a_Position
	"normal",        // a_Normal
	"coord",         // a_Coord
	"coord2",        // a_Coord2
	"vertex",        // a_Vertex
	"color",         // a_Color
};





ShaderProgram::ShaderProgram() 
	: program(0), vsmod(0), fsmod(0)
{
	memset(uniforms,   -1, sizeof uniforms);
	memset(attributes, -1, sizeof attributes);
}

ShaderProgram::ShaderProgram(const char* vs_file, const char* fs_file) 
	: program(0), vsf(vs_file), fsf(fs_file), vsmod(0), fsmod(0) 
{
	memset(uniforms,   -1, sizeof uniforms);
	memset(attributes, -1, sizeof attributes);
}

ShaderProgram::ShaderProgram(const std::string& vs_file, const std::string& fs_file) 
	: program(0), vsf(vs_file), fsf(fs_file), vsmod(0), fsmod(0) 
{
	memset(uniforms,   -1, sizeof uniforms);
	memset(attributes, -1, sizeof attributes);
}

ShaderProgram::~ShaderProgram()
{
	Destroy();
}

void ShaderProgram::Destroy() // just GL delete the shader program
{
	if (program)
	{
		glDeleteProgram(program);
		program = 0;
	}
}


// performs hotload if Vertex or Fragment programs changed
void ShaderProgram::HotLoad()
{
	if (shaderdir.changed())
	{
		if (file_modified(vsf) != vsmod || file_modified(fsf) != fsmod)
		{
			Destroy(); // destroy the shader to allow recompile
			if (Compile())
			{
				printf("ShaderProgram::HotLoad():\n    VP: %s\n    FP: %s\n", vsf, fsf);
			}
		}
	}
}




static void PrintError(unsigned obj);
static unsigned CompileShader(const char* sh_mem, int size, const char* idstr, GLenum type);
static unsigned CompileShaderFile(const char* sh_file, time_t& modified, GLenum type);
static unsigned LinkShaderProgram(unsigned vs, unsigned fs);

bool ShaderProgram::Compile()
{ 
	return CompileFromFile(vsf.c_str(), fsf.c_str());
}

bool ShaderProgram::CompileFromFile(const char* vs_file, const char* fs_file)
{
	if (program)
	{
		fprintf(stderr, "Shader program already compiled!\n");
		return false; // program already compiled!
	}
	vsf = vs_file;
	fsf = fs_file;
	unsigned vs = CompileShaderFile(vs_file, vsmod, GL_VERTEX_SHADER);
	unsigned fs = CompileShaderFile(fs_file, fsmod, GL_FRAGMENT_SHADER);
	if (vs && fs)
	{
		shaderdir.initialize(directory::foldername(fs_file));
		program = LinkShaderProgram(vs, fs);
		LoadVariables();
	}
	if (vs) glDeleteShader(vs);
	if (fs) glDeleteShader(fs);
	return this->program ? true : false;
}

bool ShaderProgram::CompileFromString(const char* vs_str, const char* fs_str)
{
	if (program)
	{
		fprintf(stderr, "Shader program already compiled!\n");
		return false; // program already compiled
	}
	shaderdir.close(); // close directory monitor
	vsf.clear();
	fsf.clear();
	unsigned vs = CompileShader(vs_str, strlen(vs_str), ".vs", GL_VERTEX_SHADER);
	unsigned fs = CompileShader(fs_str, strlen(fs_str), ".fs", GL_FRAGMENT_SHADER);
	if (vs && fs)
	{
		program = LinkShaderProgram(vs, fs);
		LoadVariables();
	}
	if (vs) glDeleteShader(vs);
	if (fs) glDeleteShader(fs);
	return this->program ? true : false;
}










#pragma region GLSLCompilerLinker
// Prints the error log on this gl object. Assumes that an error occurrred
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
// Compiles string into a gl shader
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
// Compiles a file into a shader
static unsigned CompileShaderFile(const char* sh_file, time_t& modified, GLenum type)
{
	if (sh_file == NULL || !sh_file[0])
	{
		fprintf(stderr, "GLSL compile error: shader file '' is invalid!\n");
		return 0;
	}

	unbuffered_file f(sh_file, READONLY);
	if (f.bad())
	{
		fprintf(stderr, "GLSL compile error: shader file '%s' not found!\n", sh_file);
		return 0;
	}
	modified     = f.time_modified();
	int buffsize = f.size_aligned();

	char* buffer = (char*)(buffsize <= 65536 ? alloca(buffsize) : malloc(buffsize));
	unsigned shader = 0;
	if (int size = f.read(buffer, buffsize)) // could we read in the data?
	{
		shader = CompileShader(buffer, size, sh_file, type);
	}
	else fprintf(stderr, "GLSL compile error: shader file '%s' could not be read to the end!\n", sh_file);
	if (buffsize > 65536) free(buffer); // free if on the heap
	return shader;
}
// Links VS and FS together into a GLSL Shader Program
static unsigned LinkShaderProgram(unsigned vs, unsigned fs)
{
	unsigned program = glCreateProgram();
	// bind attribute names to fixed locations
	for (int i = 0; i < a_MaxAttributes; ++i)
		glBindAttribLocation(program, i, AttributeMap[i]);
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	GLint link_ok; glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok)
	{
		fprintf(stderr, "GLSL linkage failure! vs:%d fs:%d\n", vs, fs);
		PrintError(program);
	}
	return program;
}
#pragma endregion











// currently bound program
const ShaderProgram* ShaderProgram::Program = NULL;


// binds this shader to the pipeline
void ShaderProgram::Bind()
{
	if (Program == this) return; // no need to change anything
	if (Program) Program->UnbindAttributes();
	Program = this;
	glUseProgram(program);
	BindAttributes();
}

// unbinds this shader from the pipeline
void ShaderProgram::Unbind()
{
	if (!Program) return; // don't trouble OpenGL with unnecessary null calls
	if (Program) Program->UnbindAttributes();
	Program = NULL;
	glUseProgram(0);
}

// binds this model-view-projection matrix to the shader
void ShaderProgram::BindMatrix(const glm::mat4& modelViewProjection)
{
	int u = Uniform(u_Transform);
	if (u == -1) return; // no transform matrix available in shader
	glUniformMatrix4fv(u, 1, GL_FALSE, (float*)&modelViewProjection);
}

// binds a raw opengl texture to this shader
void ShaderProgram::BindTexture(unsigned glTexture)
{
	int u = Uniform(u_DiffuseTex);
	if (u == -1) return; // no diffuse texture available in shader
	if (!glTexture) return; // no material loaded?
	glBindTexture(GL_TEXTURE_2D, glTexture);
	glUniform1i(u, 0); // texture 0
}

// binds a new texture to this shader
void ShaderProgram::BindTexture(const Texture* texture)
{
	int u = Uniform(u_DiffuseTex);
	if (u == -1) return; // no diffuse texture available in shader
	if (!texture || !texture->glTexture) return; // no material loaded?
	glBindTexture(GL_TEXTURE_2D, texture->glTexture);
	glUniform1i(u, 0); // texture 0
}

// binds a specific diffuse color to this shader
void ShaderProgram::BindDiffuseColor(const Vector4& colorRGBA)
{
	int u = Uniform(u_DiffuseColor);
	if (u == -1) return; // no diffuse color available in shader
	glUniform4fv(u, 1, (float*)&colorRGBA); 
}

// binds a specific background color to this shader
void ShaderProgram::BindOutlineColor(const Vector4& outlineRGBA)
{
	int u = Uniform(u_OutlineColor);
	if (u == -1) return; // no diffuse color available in shader
	glUniform4fv(u, 1, (float*)&outlineRGBA); 
}

// draws a 3D model using this shader
void ShaderProgram::Draw(const VertexBuffer* vb)
{
	if (vb) vb->Draw();
}

void ShaderProgram::LoadVariables()
{
	// brute force load all supported uniform and attribute locations
	for (int i = 0; i < u_MaxUniforms; ++i)
		uniforms[i] = (char)glGetUniformLocation(program, UniformMap[i]);
	for (int i = 0; i < a_MaxAttributes; ++i)
		attributes[i] = (char)glGetAttribLocation(program, AttributeMap[i]);
}

void ShaderProgram::BindAttributes() const
{
	for (int i = 0; i < a_MaxAttributes; ++i)
	{
		int attr = (int)attributes[i];
		if (attr != -1) glEnableVertexAttribArray(attr);	// bind attribute
	}
}

void ShaderProgram::UnbindAttributes() const
{
	for (int i = 0; i < a_MaxAttributes; ++i)
	{
		int attr = (int)attributes[i];
		if (attr != -1) glDisableVertexAttribArray(attr);	// unbind attribute
	}
}