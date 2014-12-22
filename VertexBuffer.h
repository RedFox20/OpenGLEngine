/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#pragma once
#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include "Basetypes.h"


/**
 * @brief ShaderProgram uniform slots
 */
enum ShaderUniform
{
	u_Transform,    // uniform matrix transform;      model-view-project matrix
	u_DiffuseTex,   // uniform sampler2D diffuseTex;  diffuse texture
	u_SpecularTex,  // uniform sampler2D specularTex; specular texture
	u_NormalTex,    // uniform sampler2D normalTex;   normal texture
	u_ShadowTex,    // uniform sampler2D shadowTex;   shadow texture
	u_OccludeTex,   // uniform sampler2D occludeTex;  occlusion texture for fake SSAO
	u_DiffuseColor, // uniform vec4 diffuseColor;     diffuse color 
	u_OutlineColor, // uniform vec4 outlineColor;     background or outline color
	u_MaxUniforms,  // uniform counter
};


/**
 * @brief ShaderProgram attribute slots
 */
enum ShaderAttribute
{
	a_Position,      // attribute vec3 position;  position (vec2 XY or vec3 XYZ)
	a_Normal,        // attribute vec3 normal;    normal 
	a_Coord,         // attribute vec2 coord;     texture coordinate 0
	a_Coord2,        // attribute vec2 coord2;    texture coordinate 1
	a_Vertex,        // attribute vec4 vertex;    additional generic 4D vertex
	a_Color,         // attribute vec4 color;     per-vertex coloring
	a_MaxAttributes, // attribute counter
};




#pragma pack(push, 1)
/**
 * @note Vertex description structure. Attributes count, SizeOf vertex (in bytes), Descr array 
 */
struct VertexDescr
{
	byte Count;      // number of attributes per vertex
	byte SizeOf;     // size of vertex in bytes
	struct AttribMap
	{
		byte attr; // ShaderAttribute type, such as a_Position
		byte size; // attribute element count (number of floats to bind)
	} Attribs[3];
};
#pragma pack(pop)




/**
 * @note Describes a simple 3D vertex
 * @note Attributes a_Vertex
 */
struct Vertex3
{
	float x, y, z; // vertex.xyz
	static const VertexDescr* GetVertexDescr() {
		static VertexDescr descr = { 1, sizeof(Vertex3), {{a_Vertex,3}} };
		return &descr;
	}
};

/**
 * @note Describes a vertex-colored 3D vertex
 * @note Attributes a_Vertex
 */
struct Vertex3Color
{
	float x, y, z;	// vertex.xyz
	float rgba;		// packedRGBA
	static const VertexDescr* GetVertexDescr() {
		static VertexDescr descr = { 1, sizeof(Vertex3Color), {{a_Vertex,4}} };
		return &descr;
	}
};

/**
 * @note Describes a 3D vertex with UV coordinates
 * @note Attributes a_Position, a_Coord
 */
struct Vertex3UV
{
	float x, y, z;	// attribute position.xyz
	float u, v;		// attribute coord.uv
	static const VertexDescr* GetVertexDescr() {
		static VertexDescr descr = { 2, sizeof(Vertex3UV), {{a_Position,3},{a_Coord,2}} };
		return &descr;
	}
};




/**
 * @note Describes a simple 2D vertex
 * @note Attributes a_Position
 */
struct Vertex2
{
	float x, y; // attribute position.xy
	static const VertexDescr* GetVertexDescr() {
		static VertexDescr descr = { 1, sizeof(Vertex2), {{a_Position,2}} };
		return &descr;
	}
};

/**
 * @note Describes a 2D vertex with per-vertex coloring
 * @note Attributes a_Position
 */
struct Vertex2Color
{
	float x, y; // attribute position.xy
	float rgba;	// packedRGBA
	static const VertexDescr* GetVertexDescr() {
		static VertexDescr descr = { 1, sizeof(Vertex2Color), {{a_Position,3}} };
		return &descr;
	}
};

/**
 * @note Describes a 2D vertex with per-vertex coloring
 * @note Attributes a_Position, a_Color
 */
struct Vertex2ColorUnpacked
{
	float x, y;			// attribute position.xy
	Vector4 rgba;
	static const VertexDescr* GetVertexDescr() {
		static VertexDescr descr = { 2, sizeof(Vertex2ColorUnpacked), {{a_Position,2},{a_Color,4}} };
		return &descr;
	}
};

/**
 * @note Describes a 2D vertex with UV coordinates
 * @note Attributes a_Position, a_Coord
 */
struct Vertex2UV
{
	float x, y; // attribute position.xy
	float u, v; // attribute coord.uv
	static const VertexDescr* GetVertexDescr() {
		static VertexDescr descr = { 2, sizeof(Vertex2UV), {{a_Position,2},{a_Coord,4}} };
		return &descr;
	}
};

/**
 * @note Describes a 4D vertex with 4 components
 * @note Attributes a_Vertex
 */
struct Vertex4
{
	float x, y, z, w; // attribute vertex.xyzw
	static const VertexDescr* GetVertexDescr() {
		static VertexDescr descr = { 1, sizeof(Vertex4), {{a_Vertex,4}} };
		return &descr;
	}
};



/**
 * @note The type of index.
 */
typedef unsigned int Index;

// describes the type of buffers used on a model
enum BufferType {
	BUFFER_INVALID,
	BUFFER_STATIC,
	BUFFER_DYNAMIC,
	BUFFER_STREAM,
};

// describes the drawing styles used on a model
enum DrawStyle {
	DRAW_INVALID,
	DRAW_TRIANGLES,
	DRAW_TRIANGLE_STRIP,
	DRAW_TRIANGLE_FAN,
	DRAW_LINES,
	DRAW_LINE_STRIP,
};

// describes buffer mapping policies
enum BufferMapType {
	MAP_RO,	// map for read-only operations
	MAP_WO, // map for write-only operations
	MAP_RW, // map for read/write operations
};




/**
 * @brief A simple VertexBuffer for rendering any kind of Vertex
 */
struct VertexBuffer
{
	const VertexDescr* Descr;	// vertex description
	uint VBO;				// opengl vertex buffer
	int VertexCount;			// number of vertices in the vertex buffer
	uchar Type;					// type of buffer (static, dynamic, ...)
	uchar Style;				// drawing style (triangles, line)


	/**
	 * @brief Creates a default initialized VertexBuffer
	 */
	inline VertexBuffer() : VBO(0), Descr(0), VertexCount(0), Type(BUFFER_INVALID), Style(DRAW_INVALID) {}
	

	/**
	 * @brief Creates a new VertexBuffer, default type is STATIC TRIANGLES
	 * @param vertexType Defines the vertex type of this VertexBuffer
	 * @param btype Type of buffer to use
	 * @param dstyle Drawing style
	 */
template<class VERTEX> 
	inline VertexBuffer(const VERTEX& vertexType, BufferType btype = BUFFER_STATIC, DrawStyle dstyle = DRAW_TRIANGLES) 
		: VBO(0), VertexCount(0)
	{
		Create(VERTEX::GetVertexDescr(), btype, dstyle);
	}


	/**
	 * @brief Destroys the VertexBuffer if needed
	 */
	virtual ~VertexBuffer();


	/**
	 * @brief Copying actually produces move semantics
	 */
	VertexBuffer& operator=(VertexBuffer& vb);




	/**
	 * @brief Creates a new VertexBuffer, default type is STATIC TRIANGLES
	 * @param btype Type of buffer to use
	 * @param dstyle Drawing style
	 * @return TRUE on success. Fails if buffer has been created.
	 */
template<class VERTEX>
	inline bool Create(BufferType btype = BUFFER_STATIC, DrawStyle dstyle = DRAW_TRIANGLES)
	{
		return Create(VERTEX::GetVertexDescr(), btype, dstyle);
	}


	/**
	 * @brief Creates a new VertexBuffer, default type is STATIC TRIANGLES
	 * @param vdescr Vertex layout description object
	 * @param btype Type of buffer to use
	 * @param dstyle Drawing style
	 * @return TRUE on success. Fails if buffer has been created.
	 */
	bool Create(const VertexDescr* vdescr, BufferType btype = BUFFER_STATIC, DrawStyle dstyle = DRAW_TRIANGLES);
	

	/**
	 * @brief Destroys the VertexBuffer and its buffers
	 */
	virtual void Destroy();




	/**
	 * @brief Binds this VertexBuffer's attributes to the shader
	 * @note THis is internal. You should never call this.
	 */
	void _Bind() const;
	

	/**
	 * @brief Draws the VertexBuffer on the screen.
	 */
	virtual void Draw() const;


	/**
	 * @brief Draws a segment of the VertexBuffer on the screen.
	 * @param start Starting offset index
	 * @param count Number of elements to draw
	 */
	virtual void Draw(int start, int count) const;


	/**
	 * @return TRUE if the VertexBuffer has been initialized
	 */
	inline bool IsCreated() const { return VBO ? true : false; }

	
	/**
	 * @brief Maps this VertexBuffer into client memory
	 * @param mapType Type of mapping to use R/W/RW
	 * @note UnmapVBO must be called after calling MapVBO (!)
	 * @return Mapped address to client memory
	 */
	void* MapVBO(BufferMapType mapType = MAP_RO);


	/**
	 * @brief Unmaps the VertexBuffer from client memory
	 * @note Must be called after using MapVBO (!)
	 */
	void UnmapVBO();


	/**
	 * @brief Buffers data into the VertexBuffer
	 * @param vertices Array of vertices to buffer
	 * @param vertexCount Number of vertices in the array
	 */
	void BufferVertices(const void* vertices, int vertexCount);


	/**
	 * @brief Updates buffer with new values and size
	 * @note If vertex array is not big enough, its expanded to fit all the verts
	 * @param vertices Array of vertices to buffer
	 * @param vertexCount Number of vertices in the array
	 */
	void UpdateVertices(const void* vertices, int vertexCount);


	/**
	 * @brief Inserts new vertices at the specified index
	 * @note The vertex array will be expanded to fit all the vertices
	 * @param vertices Array of vertices to insert
	 * @param vertexCount Number of vertices in the array
	 * @param offsetIndex Offset index where to insert
	 */
	void InsertVertices(const void* vertices, int vertexCount, int offsetIndex = 0);

	/**
	 * @brief Appends vertices to the end of the buffer
	 * @note The vertex array will be expanded to fit all the vertices
	 * @param vertices Array of vertices to append
	 * @param vertexCount Number of vertices in the arrray
	 */
	void AppendVertices(const void* vertices, int vertexCount);
};






/**
 * @brief A vertex-index buffer for rendering any type of indexed vertices
 */
struct VertexIndexBuffer : public VertexBuffer
{
	uint IBO;			// opengl index buffer
	int IndexCount;	// number of indices in the index buffer


	/**
	 * @brief Creates an uninitialized vertex-index buffer
	 */
	inline VertexIndexBuffer() : IBO(0), IndexCount(0) {}
	

	/**
	 * @brief Creates a new VertexBuffer, default type is STATIC TRIANGLES
	 */
template<class VERTEX> 
	inline VertexIndexBuffer(const VERTEX& vertexType, BufferType btype = BUFFER_STATIC, DrawStyle dstyle = DRAW_TRIANGLES) 
		: IBO(0), IndexCount(0)
	{
		Create(VERTEX::GetVertexDescr(), btype, dstyle);
	}


	/**
	 * @brief Destroys the VertexIndexBuffer if needed
	 */
	virtual ~VertexIndexBuffer();


	/**
	 * @brief Copying actually produces move semantics
	 */
	VertexIndexBuffer& operator=(VertexIndexBuffer& vib);




	/**
	 * @brief Destroys the VertexIndexBuffer
	 */
	virtual void Destroy() override; 


	/**
	 * @brief Draws this VertexIndexBuffer
	 */
	virtual void Draw() const override;
	

	/**
	 * @brief Draws a segment of the VertexIndexBuffer on the screen.
	 * @param start Starting offset index
	 * @param count Number of elements to draw
	 */
	virtual void Draw(int start, int count) const;

	
	/**
	 * @return TRUE if both Vertex and Index buffers were initialized
	 */
	inline operator bool() const { return VBO && IBO ? true : false; }


	/**
	 * @brief Maps this IndexBuffer into client memory
	 * @param mapType Type of mapping to use R/W/RW
	 * @note UnmapIBO must be called after MapIBO()
	 * @return Mapped address to client memory
	 */
	void* MapIBO(BufferMapType mapType = MAP_RO);


	/**
	 * @brief Unmaps the IndexBuffer from client memory
	 * @note Must be called after MapIBO()
	 */
	void UnmapIBO();


	/**
	 * @brief Buffers indices into the index buffer
	 * @param indices Array of indices to buffer into the IBO
	 * @param indexCount Number of indices in the buffer
	 */
	void BufferIndices(const Index* indices, int indexCount);


	/**
	 * @brief Buffers subdata insider the indexbuffer
	 * @param indices Array of indices to buffer into the IBO
	 * @param indexCount Number of indices in the array
	 * @param offsetIndex Offset index of the first index in IBO to replace
	 */
	void BufferSubIndices(const Index* indices, int indexCount, int offsetIndex = 0);
};

#endif // VERTEXBUFFER_H